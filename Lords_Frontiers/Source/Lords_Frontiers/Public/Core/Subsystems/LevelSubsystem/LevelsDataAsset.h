// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "LevelsDataAsset.generated.h"

USTRUCT()
struct FGameplayLevelData
{
	GENERATED_BODY()

	UPROPERTY( EditDefaultsOnly )
	TSoftObjectPtr<UWorld> Level;

	UPROPERTY(
	    EditDefaultsOnly,
	    meta =
	        ( Tooltip = "Warning: changes are applied only if GameSaverConfig bClearAllSaveDataOnGameStart is checked" )
	)
	bool Unlocked = false;
};

/** (Gregory-hub)
 * Contains all game levels */
UCLASS()
class LORDS_FRONTIERS_API ULevelsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditDefaultsOnly, Category = "Settings|Levels" )
	TSoftObjectPtr<UWorld> MainMenuLevel;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Levels" )
	TSoftObjectPtr<UWorld> LevelChoosingLevel;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Levels" )
	TArray<FGameplayLevelData> GameplayLevels;
};
