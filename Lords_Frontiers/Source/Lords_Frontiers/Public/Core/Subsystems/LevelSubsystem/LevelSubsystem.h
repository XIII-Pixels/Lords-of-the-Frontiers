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
	void LoadMainMenu() const;
	void LoadLevelChoosingLevel() const;
	void LoadGameplayLevel( int index );
	void LoadNextLevel();

	void SetLevels( TSoftObjectPtr<ULevelsDataAsset> levels );
	void ResetSavedLevelStatuses() const;

	ELevelStatus GetLevelStatus( int index ) const;

	void UnlockNextLevel() const;

	int GetCurrentLevelIndex() const
	{
		return CurrentLevelIndex_;
	}

	/** Fills outConfig with the camera config of the level that is currently loaded.
	 * Returns false when no gameplay level is active (e.g. launched directly into a map). */
	bool GetCurrentLevelCameraConfig( FLevelCameraConfig& outConfig ) const;

protected:
	UPROPERTY()
	TObjectPtr<ULevelsDataAsset> Levels_;

	void LoadLevel( TSoftObjectPtr<UWorld> level, const FString& errorMessage = "", bool bUseTransition = true ) const;

	int CurrentLevelIndex_ = -1;
};
