// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LoopingSoundConfig.h"

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "MusicDataAsset.generated.h"

/** (Gregory-hub)
 * Music for different turns for one level */
USTRUCT( BlueprintType )
struct FMusicForLevel
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere )
	FLoopingSoundConfig Building;

	UPROPERTY( EditAnywhere )
	FLoopingSoundConfig Battle;
};

/** (Gregory-hub)
 * Music config */
UCLASS()
class LORDS_FRONTIERS_API UMusicDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	const FLoopingSoundConfig& MainMenuMusic() const
	{
		return MainMenuMusic_;
	}

	const FLoopingSoundConfig& WinMusic() const
	{
		return WinMusic_;
	}

	const FLoopingSoundConfig& LoseMusic() const
	{
		return LoseMusic_;
	}

	const FMusicForLevel* MusicForLevel( TSoftObjectPtr<UWorld> level ) const;

	float BattleMusicVolumeScale() const
	{
		return BattleMusicVolumeScale_;
	}

private:
	/** Extra multiplier applied on top of the per-track Volume for battle music only. */
	UPROPERTY( EditAnywhere, Category = "Settings", meta = ( ClampMin = 0.0f, ClampMax = 1.0f ) )
	float BattleMusicVolumeScale_ = 0.5f;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	FLoopingSoundConfig MainMenuMusic_;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	FLoopingSoundConfig WinMusic_;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	FLoopingSoundConfig LoseMusic_;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	TMap<TSoftObjectPtr<UWorld>, FMusicForLevel> MusicForLevels_;
};
