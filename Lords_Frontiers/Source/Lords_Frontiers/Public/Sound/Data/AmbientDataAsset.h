// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Sound/Data/LoopingSoundConfig.h"

#include "AmbientDataAsset.generated.h"

/** (Gregory-hub)
 * Ambient sounds for one level */
USTRUCT( BlueprintType )
struct FAmbientForLevel
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere )
	TArray<FLoopingSoundConfig> AmbientEntries;
};

/** (Gregory-hub)
 * Ambient config */
UCLASS()
class LORDS_FRONTIERS_API UAmbientDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	const FLoopingSoundConfig& WindAmbient() const
	{
		return WindAmbient_;
	}

	TMap<TSoftObjectPtr<UWorld>, FAmbientForLevel> AmbientForLevels() const
	{
		return AmbientForLevels_;
	}

private:
	UPROPERTY( EditAnywhere, Category = "Settings" )
	FLoopingSoundConfig WindAmbient_;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	TMap<TSoftObjectPtr<UWorld>, FAmbientForLevel> AmbientForLevels_;
};
