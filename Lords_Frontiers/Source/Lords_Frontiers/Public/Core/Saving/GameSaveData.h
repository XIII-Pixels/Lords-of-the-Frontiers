// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"

#include "GameSaveData.generated.h"

UENUM()
enum class ELevelStatus
{
	Undefined,
	Locked,
	Unlocked,
	Completed
};

/** (Gregory-hub)
 * Saved game data */
UCLASS()
class LORDS_FRONTIERS_API UGameSaveData : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TMap<FString, ELevelStatus> StatusLevels;
};
