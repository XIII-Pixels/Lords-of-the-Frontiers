// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Subsystems/TransitionSubsystem/TransitionSubsystem.h"

#include "Core/Subsystems/TransitionSubsystem/TransitionConfig.h"
#include "Core/Subsystems/TransitionSubsystem/TransitionOverlayWidget.h"

#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
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
	// Cover is up. Preload the target level asynchronously so the game thread keeps ticking and
	// idle_screen animates instead of freezing on a blocking OpenLevel; travel once it is ready.
	const FSoftObjectPath levelAsset = ResolvePendingLevelAsset();
	UTransitionOverlayWidget* overlay = ActiveOverlay_.Get();
	if ( !levelAsset.IsValid() || !overlay )
	{
		// Unresolved target (e.g. a short map name) or no overlay: travel straight away.
		PerformOpenLevel();
		return;
	}

	// Bind and start idle before kicking the load so a synchronous completion can't strand us.
	overlay->OnIdleFinished.AddUObject( this, &UTransitionSubsystem::HandleIdleSettledBeforeTravel );
	overlay->PlayIdleLoop();

	TWeakObjectPtr<UTransitionSubsystem> weakThis( this );
	PreloadHandle_ = StreamableManager_.RequestAsyncLoad(
	    levelAsset,
	    FStreamableDelegate::CreateLambda(
	        [weakThis]()
	        {
		        if ( UTransitionSubsystem* self = weakThis.Get() )
		        {
			        self->HandleLevelPreloaded();
		        }
	        } ) );

	if ( !PreloadHandle_.IsValid() )
	{
		// Couldn't start the async load: settle idle and travel now (synchronous fallback).
		HandleLevelPreloaded();
	}
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

	overlay->OnEndFinished.AddUObject( this, &UTransitionSubsystem::HandleEndFinished );

	// The new map is already loaded and the screen is covered; reveal it. idle_screen already
	// animated on the previous overlay while the package streamed in (see HandleStartFinished).
	overlay->PlayEnd();
}

void UTransitionSubsystem::HandleLevelPreloaded()
{
	// The target package is in memory now: wind idle down to the nearest edge, then travel (which
	// is fast — no disk IO). If the overlay vanished, travel straight away.
	if ( UTransitionOverlayWidget* overlay = ActiveOverlay_.Get() )
	{
		overlay->StopIdleAtNearestEdge();
	}
	else
	{
		PerformOpenLevel();
	}
}

void UTransitionSubsystem::HandleIdleSettledBeforeTravel()
{
	PerformOpenLevel();
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

	// Release the preloaded package; if we travelled, the new world is referenced by the engine.
	PreloadHandle_.Reset();
}

FSoftObjectPath UTransitionSubsystem::ResolvePendingLevelAsset() const
{
	switch ( PendingTarget_ )
	{
	case EPendingTarget::BySoftPtr:
		return PendingSoftLevel_.IsNull() ? FSoftObjectPath() : PendingSoftLevel_.ToSoftObjectPath();
	case EPendingTarget::ByName:
	{
		// Only a full package path can be preloaded as an asset; short map names (and PIE worlds)
		// fall back to a direct synchronous OpenLevel.
		const FString name = PendingLevelName_.ToString();
		if ( FPackageName::IsValidLongPackageName( name ) )
		{
			return FSoftObjectPath( name + TEXT( "." ) + FPackageName::GetShortName( name ) );
		}
		return FSoftObjectPath();
	}
	default:
		return FSoftObjectPath();
	}
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
