// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"

#include "TransitionSubsystem.generated.h"

class UTransitionConfig;
class UTransitionOverlayWidget;

/**
 * Drives the UMG loading-screen transition (start_screen -> idle_screen -> end_screen).
 *
 * Lives on the GameInstance so it survives OpenLevel. The flow is:
 *   1. Create the overlay on the current level and play start_screen (covers the screen).
 *   2. When start_screen finishes, OpenLevel the target map (the old overlay is destroyed).
 *   3. On PostLoadMapWithWorld the new map is already loaded, so the idle hold is skipped and
 *      end_screen plays straight away (idle_screen is reserved for an async-loading path).
 *   4. Play end_screen to reveal the new level, then remove the overlay.
 *
 * If no overlay class is configured, the target map is opened immediately with no effect,
 * so level loading never breaks.
 */
UCLASS()
class LORDS_FRONTIERS_API UTransitionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize( FSubsystemCollectionBase& collection ) override;
	virtual void Deinitialize() override;

	/** Assigns the transition config. Call once from the GameInstance. */
	void SetConfig( UTransitionConfig* config );

	/** Starts a transition that opens the level identified by name. */
	void TransitionToLevelByName( FName levelName );

	/** Starts a transition that opens the given (soft) level asset. */
	void TransitionToSoftLevel( const TSoftObjectPtr<UWorld>& level );

	/** Starts a transition that reloads the current level for a fresh run. */
	void TransitionRestart();

	/** True while a transition is in progress. */
	bool IsTransitioning() const
	{
		return bIsTransitioning_;
	}

private:
	/** How the pending level should be opened once the screen is covered. */
	enum class EPendingTarget : uint8
	{
		None,
		ByName,
		BySoftPtr
	};

	void BeginTransition();
	void PerformOpenLevel();
	void HandlePostLoadMap( UWorld* loadedWorld );
	void HandleNewLevelReady();
	void FinishTransition();

	void HandleStartFinished();
	void HandleLevelPreloaded();
	void HandleIdleSettledBeforeTravel();
	void HandleEndFinished();

	FSoftObjectPath ResolvePendingLevelAsset() const;
	void OpenPendingLevelNow() const;
	bool UseOverlayPath() const;
	UTransitionOverlayWidget* EnsureOverlay();

	UPROPERTY()
	TObjectPtr<UTransitionConfig> Config_;

	UPROPERTY()
	TWeakObjectPtr<UTransitionOverlayWidget> ActiveOverlay_;

	EPendingTarget PendingTarget_ = EPendingTarget::None;
	FName PendingLevelName_;
	TSoftObjectPtr<UWorld> PendingSoftLevel_;

	bool bIsTransitioning_ = false;
	bool bPendingReveal_ = false;

	FDelegateHandle PostLoadMapHandle_;

	/** Streams the target level package in while idle_screen animates, so the final OpenLevel
	 *  doesn't block the game thread on disk IO. */
	FStreamableManager StreamableManager_;
	TSharedPtr<FStreamableHandle> PreloadHandle_;
};
