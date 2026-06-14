// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"

#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"

#include "TransitionSubsystem.generated.h"

class FTimerManager;
class UNiagaraComponent;
class UNiagaraSystem;
class UTransitionConfig;
class UTransitionOverlayWidget;
class USceneComponent;

/**
 * Drives the three-stage Niagara level-transition effect (appear -> wait -> disappear).
 *
 * Lives on the GameInstance so it survives OpenLevel. The flow is:
 *   1. Spawn the appear effect on the current level (covers the screen).
 *   2. After AppearDuration, OpenLevel the requested map (the old world is destroyed).
 *   3. On PostLoadMapWithWorld, spawn the wait effect on the new level (hold).
 *   4. After WaitDuration, spawn the disappear effect (reveal).
 *   5. After DisappearDuration, the transition is finished.
 *
 * If no config / appear effect is assigned, the requested map is opened
 * immediately with no effect, so level loading never breaks.
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

	void RunAppearThenOpen();
	void PerformOpenLevel();
	void HandlePostLoadMap( UWorld* loadedWorld );
	void HandleNewLevelReady();
	void BeginReveal();
	void FinishTransition();

	void OpenPendingLevelNow() const;
	void StopActiveVFX();
	UNiagaraComponent* SpawnAttachedToView( UNiagaraSystem* system, bool bAutoDestroy ) const;
	USceneComponent* GetViewAttachComponent() const;
	FTimerManager* GetActiveTimerManager() const;

	/** True when the transition is rendered through the UMG overlay (over everything). */
	bool UseOverlayPath() const;
	/** Creates the overlay widget on the current viewport if it does not exist yet. */
	void EnsureOverlay();

	UPROPERTY()
	TObjectPtr<UTransitionConfig> Config_;

	UPROPERTY()
	TWeakObjectPtr<UNiagaraComponent> ActiveVFX_;

	UPROPERTY()
	TWeakObjectPtr<UTransitionOverlayWidget> ActiveOverlay_;

	EPendingTarget PendingTarget_ = EPendingTarget::None;
	FName PendingLevelName_;
	TSoftObjectPtr<UWorld> PendingSoftLevel_;

	bool bIsTransitioning_ = false;
	bool bPendingReveal_ = false;

	FDelegateHandle PostLoadMapHandle_;
	FTimerHandle StageTimerHandle_;
};
