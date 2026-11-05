// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LevelsDataAsset.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "CoreMinimal.h"

#include "LevelSubsystem.generated.h"

enum class ELevelStatus;

/** (Gregory-hub)
 * Subsystem for level loading */
UCLASS()
class LORDS_FRONTIERS_API ULevelSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void LoadMainMenu( bool bUseTransition = true ) const;
	void LoadLevelChoosingLevel() const;
	void LoadGameplayLevel( int index );

	// Loads the gameplay level after the current one. Returns false if there is none.
	bool LoadNextLevel();

	void SetLevels( TSoftObjectPtr<ULevelsDataAsset> levels );
	void ResetSavedLevelStatuses() const;

	ELevelStatus GetLevelStatus( int index ) const;

	void UnlockNextLevel() const;

	int GetCurrentLevelIndex() const
	{
		return ResolveCurrentLevelIndex();
	}

	/** Fills outConfig with the camera config of the level that is currently loaded.
	 * Returns false when no gameplay level is active (e.g. launched directly into a map). */
	bool GetCurrentLevelCameraConfig( FLevelCameraConfig& outConfig ) const;

protected:
	UPROPERTY()
	TObjectPtr<ULevelsDataAsset> Levels_;

	void LoadLevel( TSoftObjectPtr<UWorld> level, const FString& errorMessage = "", bool bUseTransition = true ) const;

	int CurrentLevelIndex_ = -1;

	// Index of the gameplay level that is open right now, found by the world's map name.
	// CurrentLevelIndex_ alone is only set when a level is opened through LoadGameplayLevel /
	// LoadNextLevel, so levels opened otherwise (PIE, "Next level" button) got another level's
	// camera config (#721). Returns INDEX_NONE if the open map is not a gameplay level.
	int32 ResolveCurrentLevelIndex() const;
};
