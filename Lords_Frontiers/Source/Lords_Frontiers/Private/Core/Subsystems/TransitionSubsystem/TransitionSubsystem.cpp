// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Subsystems/TransitionSubsystem/TransitionSubsystem.h"

#include "Core/Subsystems/TransitionSubsystem/TransitionConfig.h"
#include "Core/Subsystems/TransitionSubsystem/TransitionOverlayWidget.h"

#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC( LogTransition, Log, All );

void UTransitionSubsystem::Initialize( FSubsystemCollectionBase& collection )
{
	Super::Initialize( collection );

	PostLoadMapHandle_ =
	    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject( this, &UTransitionSubsystem::HandlePostLoadMap );
}

void UTransitionSubsystem::Deinitialize()
{
	if ( PostLoadMapHandle_.IsValid() )
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove( PostLoadMapHandle_ );
		PostLoadMapHandle_.Reset();
	}

	if ( UTransitionOverlayWidget* overlay = ActiveOverlay_.Get() )
	{
		overlay->RemoveFromParent();
	}
	ActiveOverlay_.Reset();

	Super::Deinitialize();
}

void UTransitionSubsystem::SetConfig( UTransitionConfig* config )
{
	Config_ = config;
}

void UTransitionSubsystem::TransitionToLevelByName( FName levelName )
{
	if ( bIsTransitioning_ )
	{
		return;
	}

	PendingTarget_ = EPendingTarget::ByName;
	PendingLevelName_ = levelName;
	PendingSoftLevel_.Reset();
	BeginTransition();
}

void UTransitionSubsystem::TransitionToSoftLevel( const TSoftObjectPtr<UWorld>& level )
{
	if ( bIsTransitioning_ )
	{
		return;
	}

	PendingTarget_ = EPendingTarget::BySoftPtr;
	PendingSoftLevel_ = level;
	PendingLevelName_ = NAME_None;
	BeginTransition();
}

void UTransitionSubsystem::TransitionRestart()
{
	UWorld* world = GetWorld();
	if ( !world )
	{
		UE_LOG( LogTransition, Warning, TEXT( "TransitionRestart: world is null." ) );
		return;
	}

	const FString mapName = UWorld::RemovePIEPrefix( world->GetMapName() );
	TransitionToLevelByName( FName( *mapName ) );
}

void UTransitionSubsystem::BeginTransition()
{
	if ( !UseOverlayPath() )
	{
		// No overlay configured: just load the level so behaviour never breaks.
		OpenPendingLevelNow();
		return;
	}

	bIsTransitioning_ = true;

	if ( Config_->bUnpauseOnTransition )
	{
		if ( UWorld* world = GetWorld() )
		{
			UGameplayStatics::SetGamePaused( world, false );
		}
	}

	if ( UTransitionOverlayWidget* overlay = EnsureOverlay() )
	{
		overlay->OnStartFinished.AddUObject( this, &UTransitionSubsystem::HandleStartFinished );
		overlay->PlayStart();
	}
	else
	{
		PerformOpenLevel();
	}
}

void UTransitionSubsystem::HandleStartFinished()
{
	PerformOpenLevel();
}

void UTransitionSubsystem::PerformOpenLevel()
{
	// Mark the new level so HandlePostLoadMap knows it must reveal.
	bPendingReveal_ = true;

	// The start overlay lives on the world we are about to tear down; OpenLevel destroys it.
	ActiveOverlay_.Reset();

	OpenPendingLevelNow();
}

void UTransitionSubsystem::HandlePostLoadMap( UWorld* loadedWorld )
{
	if ( !bPendingReveal_ )
	{
		return;
	}

	bPendingReveal_ = false;
	UE_LOG( LogTransition, Log, TEXT( "Revealing new level '%s'." ), *GetNameSafe( loadedWorld ) );
	HandleNewLevelReady();
}

void UTransitionSubsystem::HandleNewLevelReady()
{
	UTransitionOverlayWidget* overlay = EnsureOverlay();
	if ( !overlay )
	{
		FinishTransition();
		return;
	}

	overlay->OnIdleFinished.AddUObject( this, &UTransitionSubsystem::HandleIdleFinished );
	overlay->OnEndFinished.AddUObject( this, &UTransitionSubsystem::HandleEndFinished );

	overlay->PlayIdleLoop();

	// The map is loaded by the time PostLoadMapWithWorld fires, so loading is "done":
	// let the current idle cycle finish, then reveal. (Idle still plays at least one full
	// loop before end_screen.)
	overlay->StopIdleAfterCycle();
}

void UTransitionSubsystem::HandleIdleFinished()
{
	if ( UTransitionOverlayWidget* overlay = ActiveOverlay_.Get() )
	{
		overlay->PlayEnd();
	}
	else
	{
		FinishTransition();
	}
}

void UTransitionSubsystem::HandleEndFinished()
{
	FinishTransition();
}

void UTransitionSubsystem::FinishTransition()
{
	if ( UTransitionOverlayWidget* overlay = ActiveOverlay_.Get() )
	{
		overlay->RemoveFromParent();
	}
	ActiveOverlay_.Reset();

	bIsTransitioning_ = false;
	PendingTarget_ = EPendingTarget::None;
	PendingSoftLevel_.Reset();
	PendingLevelName_ = NAME_None;
}

void UTransitionSubsystem::OpenPendingLevelNow() const
{
	UWorld* world = GetWorld();
	if ( !world )
	{
		UE_LOG( LogTransition, Warning, TEXT( "OpenPendingLevelNow: world is null." ) );
		return;
	}

	switch ( PendingTarget_ )
	{
	case EPendingTarget::ByName:
		UGameplayStatics::OpenLevel( world, PendingLevelName_ );
		break;
	case EPendingTarget::BySoftPtr:
		if ( !PendingSoftLevel_.IsNull() )
		{
			UGameplayStatics::OpenLevelBySoftObjectPtr( world, PendingSoftLevel_ );
		}
		break;
	default:
		break;
	}
}

bool UTransitionSubsystem::UseOverlayPath() const
{
	return Config_ && Config_->OverlayWidgetClass != nullptr;
}

UTransitionOverlayWidget* UTransitionSubsystem::EnsureOverlay()
{
	if ( ActiveOverlay_.IsValid() )
	{
		return ActiveOverlay_.Get();
	}

	UWorld* world = GetWorld();
	if ( !world || !Config_ || !Config_->OverlayWidgetClass )
	{
		return nullptr;
	}

	if ( UTransitionOverlayWidget* overlay =
	         CreateWidget<UTransitionOverlayWidget>( world, Config_->OverlayWidgetClass ) )
	{
		overlay->AddToViewport( Config_->OverlayZOrder );
		ActiveOverlay_ = overlay;
		return overlay;
	}

	return nullptr;
}
