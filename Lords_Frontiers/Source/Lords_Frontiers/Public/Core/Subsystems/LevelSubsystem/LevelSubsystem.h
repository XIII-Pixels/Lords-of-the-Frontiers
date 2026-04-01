// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LevelsDataAsset.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "CoreMinimal.h"

#include "LevelSubsystem.generated.h"

/** (Gregory-hub)
 * Subsystem for level loading */
UCLASS()
class LORDS_FRONTIERS_API ULevelSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void LoadMainMenu() const;
	void LoadRunLevel() const;
	void LoadWinLevel() const;
	void LoadLoseLevel() const;

	void SetupLevels( TSoftObjectPtr<ULevelsDataAsset> levels );

	UPROPERTY()
	bool bShowTutorialOnNextLevel_ = true;

	void SetShowTutorialOnNextLevel( bool bValue )
	{
		bShowTutorialOnNextLevel_ = bValue;
	}

	bool ConsumeShowTutorialOnNextLevel()
	{
		const bool result = bShowTutorialOnNextLevel_; bShowTutorialOnNextLevel_ = false;
		return result;
	}

protected:
	UPROPERTY()
	TObjectPtr<ULevelsDataAsset> Levels_;

	void LoadLevel( TSoftObjectPtr<UWorld> level, const FString& errorMessage = "" ) const;
};
