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

	void SetupLevels( TSubclassOf<ULevelsDataAsset> levelsClass );

protected:
	UPROPERTY()
	TObjectPtr<ULevelsDataAsset> Levels_;

private:
	void OpenLevel(TSoftObjectPtr<UWorld> level) const;
};
