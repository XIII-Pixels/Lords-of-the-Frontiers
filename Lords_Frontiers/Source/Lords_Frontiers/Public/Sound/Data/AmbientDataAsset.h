// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Sound/Data/ContinuousSound.h"

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "AmbientDataAsset.generated.h"

/** (Gregory-hub)
 * Ambient sounds for one level */
USTRUCT( BlueprintType )
struct FAmbientForLevel
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere )
	TArray<FContinuousSound> AmbientEntries;
};

/** (Gregory-hub)
 * Ambient config */
UCLASS()
class LORDS_FRONTIERS_API UAmbientDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	const FContinuousSound& WindAmbient() const
	{
		return WindAmbient_;
	}

	TMap<TSoftObjectPtr<UWorld>, FAmbientForLevel> AmbientForLevels() const
	{
		return AmbientForLevels_;
	}

private:
	UPROPERTY( EditAnywhere, Category = "Settings" )
	FContinuousSound WindAmbient_;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	TMap<TSoftObjectPtr<UWorld>, FAmbientForLevel> AmbientForLevels_;
};
