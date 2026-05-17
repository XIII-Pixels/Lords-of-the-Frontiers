// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/AudioComponent.h"
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

	UPROPERTY(
	    EditAnywhere,
	    meta = ( ClampMin = 0.0f, ClampMax = 1.0f, ToolTip = "Volume on min camera zoom. Does not affect wind" )
	)
	float ZoomMinVolume = 0.0f;

	UPROPERTY( EditAnywhere )
	EAudioFaderCurve ZoomVolumeFaderCurve;

	UPROPERTY( EditAnywhere )
	float AdjustVolumeDuration = 0.0f;
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

	const FAmbientForLevel* AmbientForLevel( TSoftObjectPtr<UWorld> level ) const;

private:
	UPROPERTY( EditAnywhere, Category = "Settings" )
	FLoopingSoundConfig WindAmbient_;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	TMap<TSoftObjectPtr<UWorld>, FAmbientForLevel> AmbientForLevels_;
};
