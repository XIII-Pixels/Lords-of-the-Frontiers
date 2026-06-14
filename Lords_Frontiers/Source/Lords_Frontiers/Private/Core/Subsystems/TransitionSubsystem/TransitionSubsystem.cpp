// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Subsystems/TransitionSubsystem/TransitionSubsystem.h"

#include "Core/Subsystems/TransitionSubsystem/TransitionConfig.h"
#include "Core/Subsystems/TransitionSubsystem/TransitionOverlayWidget.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"

#include "Blueprint/UserWidget.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
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

	StopActiveVFX();

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
	RunAppearThenOpen();
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
	RunAppearThenOpen();
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

void UTransitionSubsystem::RunAppearThenOpen()
{
	const bool bOverlay = UseOverlayPath();
	if ( !bOverlay && ( !Config_ || !Config_->AppearVFX ) )
	{
		// No effect configured: just load the level so behaviour never breaks.
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

	if ( bOverlay )
	{
		EnsureOverlay();
		if ( UTransitionOverlayWidget* overlay = ActiveOverlay_.Get() )
		{
			overlay->PlayAppear( Config_->AppearVFX );
		}
	}
	else
	{
		StopActiveVFX();
		ActiveVFX_ = SpawnAttachedToView( Config_->AppearVFX, /*bAutoDestroy*/ true );
	}

	if ( Config_->AppearDuration > 0.0f )
	{
		if ( FTimerManager* timerManager = GetActiveTimerManager() )
		{
			timerManager->SetTimer(
			    StageTimerHandle_, this, &UTransitionSubsystem::PerformOpenLevel, Config_->AppearDuration, false
			);
			return;
		}
	}

	PerformOpenLevel();
}

void UTransitionSubsystem::PerformOpenLevel()
{
	// Mark the new level so HandlePostLoadMap knows it must reveal.
	bPendingReveal_ = true;

	// The appear visuals live on the world we are about to tear down; OpenLevel destroys
	// both the world Niagara component and the viewport overlay widget.
	ActiveVFX_.Reset();
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
	if ( UseOverlayPath() )
	{
		// Recreate the overlay on the new level's viewport, still covering the screen.
		EnsureOverlay();
		if ( UTransitionOverlayWidget* overlay = ActiveOverlay_.Get() )
		{
			overlay->PlayWait( Config_->WaitVFX );
		}
	}
	else if ( Config_ && Config_->WaitVFX )
	{
		StopActiveVFX();
		ActiveVFX_ = SpawnAttachedToView( Config_->WaitVFX, /*bAutoDestroy*/ false );
	}

	const float waitDuration = Config_ ? Config_->WaitDuration : 0.0f;
	if ( waitDuration > 0.0f )
	{
		if ( FTimerManager* timerManager = GetActiveTimerManager() )
		{
			timerManager->SetTimer( StageTimerHandle_, this, &UTransitionSubsystem::BeginReveal, waitDuration, false );
			return;
		}
	}

	BeginReveal();
}

void UTransitionSubsystem::BeginReveal()
{
	if ( UseOverlayPath() )
	{
		if ( UTransitionOverlayWidget* overlay = ActiveOverlay_.Get() )
		{
			overlay->PlayDisappear( Config_->DisappearVFX );
		}
	}
	else
	{
		// Stop the looping wait effect before revealing.
		StopActiveVFX();

		if ( Config_ && Config_->DisappearVFX )
		{
			ActiveVFX_ = SpawnAttachedToView( Config_->DisappearVFX, /*bAutoDestroy*/ true );
		}
	}

	const float disappearDuration = Config_ ? Config_->DisappearDuration : 0.0f;
	if ( disappearDuration > 0.0f )
	{
		if ( FTimerManager* timerManager = GetActiveTimerManager() )
		{
			timerManager->SetTimer(
			    StageTimerHandle_, this, &UTransitionSubsystem::FinishTransition, disappearDuration, false
			);
			return;
		}
	}

	FinishTransition();
}

void UTransitionSubsystem::FinishTransition()
{
	StopActiveVFX();

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

bool UTransitionSubsystem::UseOverlayPath() const
{
	return Config_ && Config_->OverlayWidgetClass != nullptr;
}

void UTransitionSubsystem::EnsureOverlay()
{
	if ( ActiveOverlay_.IsValid() )
	{
		return;
	}

	UWorld* world = GetWorld();
	if ( !world || !Config_ || !Config_->OverlayWidgetClass )
	{
		return;
	}

	if ( UTransitionOverlayWidget* overlay =
	         CreateWidget<UTransitionOverlayWidget>( world, Config_->OverlayWidgetClass ) )
	{
		overlay->AddToViewport( Config_->OverlayZOrder );
		ActiveOverlay_ = overlay;
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

void UTransitionSubsystem::StopActiveVFX()
{
	if ( UNiagaraComponent* vfx = ActiveVFX_.Get() )
	{
		vfx->Deactivate();
		vfx->DestroyComponent();
	}
	ActiveVFX_.Reset();
}

UNiagaraComponent* UTransitionSubsystem::SpawnAttachedToView( UNiagaraSystem* system, bool bAutoDestroy ) const
{
	if ( !system )
	{
		return nullptr;
	}

	const FVector offset = Config_ ? Config_->CameraOffset : FVector( 200.0f, 0.0f, 0.0f );
	const FVector scale = Config_ ? Config_->Scale : FVector::OneVector;

	UNiagaraComponent* component = nullptr;
	if ( USceneComponent* attachTo = GetViewAttachComponent() )
	{
		component = UNiagaraFunctionLibrary::SpawnSystemAttached(
		    system, attachTo, NAME_None, offset, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset,
		    bAutoDestroy, true, ENCPoolMethod::None
		);
	}
	else if ( UWorld* world = GetWorld() )
	{
		// No camera yet: fall back to a world-space spawn at the current view point.
		FVector location = FVector::ZeroVector;
		FRotator rotation = FRotator::ZeroRotator;
		if ( APlayerController* playerController = world->GetFirstPlayerController() )
		{
			playerController->GetPlayerViewPoint( location, rotation );
			location += rotation.RotateVector( offset );
		}
		component = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		    world, system, location, rotation, scale, bAutoDestroy, true, ENCPoolMethod::None
		);
	}

	if ( component )
	{
		if ( !scale.Equals( FVector::OneVector ) )
		{
			component->SetRelativeScale3D( scale );
		}

		// Keep the transition animating even if the game is paused, and draw it over
		// other translucent effects. Rendering over opaque world geometry additionally
		// requires "Disable Depth Test" on the effect's material (asset-side).
		component->PrimaryComponentTick.bTickEvenWhenPaused = true;
		component->SetTranslucentSortPriority( 1000 );
	}
	else
	{
		UE_LOG( LogTransition, Warning, TEXT( "SpawnAttachedToView: failed to spawn %s" ), *GetNameSafe( system ) );
	}

	return component;
}

USceneComponent* UTransitionSubsystem::GetViewAttachComponent() const
{
	UWorld* world = GetWorld();
	if ( !world )
	{
		return nullptr;
	}

	if ( APlayerController* playerController = world->GetFirstPlayerController() )
	{
		if ( APlayerCameraManager* cameraManager = playerController->PlayerCameraManager )
		{
			return cameraManager->GetTransformComponent();
		}
	}

	return nullptr;
}

FTimerManager* UTransitionSubsystem::GetActiveTimerManager() const
{
	if ( UWorld* world = GetWorld() )
	{
		return &world->GetTimerManager();
	}
	return nullptr;
}
