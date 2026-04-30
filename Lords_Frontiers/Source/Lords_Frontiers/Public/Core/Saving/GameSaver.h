// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameSaver.generated.h"

enum class ELevelStatus;

/** (Gregory-hub)
 * Class for saving game */
UCLASS()
class LORDS_FRONTIERS_API UGameSaver : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void UpdateLevelStatus( const FString& levelName, ELevelStatus levelStatus ) const;
	void UpdateCurrentLevelStatus( ELevelStatus levelStatus ) const;

	ELevelStatus GetLevelStatus( const FString& levelName ) const;
	ELevelStatus GetCurrentLevelStatus() const;

	void Clear() const;

private:
	FString SaveSlotName_ = "One Save Slot To Rule Them All";
};
