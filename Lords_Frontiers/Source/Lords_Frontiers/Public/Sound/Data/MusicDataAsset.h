// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ContinuousSound.h"

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
	FContinuousSound Building;

	UPROPERTY( EditAnywhere )
	FContinuousSound Battle;
};

/** (Gregory-hub)
 * Music config */
UCLASS()
class LORDS_FRONTIERS_API UMusicDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	const FContinuousSound& MainMenuMusic() const
	{
		return MainMenuMusic_;
	}

	const FContinuousSound& WinBattleMusic() const
	{
		return WinBattleMusic_;
	}

	const FContinuousSound& WinGameMusic() const
	{
		return WinGameMusic_;
	}

	const FContinuousSound& LoseMusic() const
	{
		return LoseMusic_;
	}

	const FMusicForLevel* MusicForLevel( TSoftObjectPtr<UWorld> level ) const;

private:
	UPROPERTY( EditAnywhere, Category = "Settings" )
	FContinuousSound MainMenuMusic_;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	FContinuousSound WinBattleMusic_;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	FContinuousSound WinGameMusic_;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	FContinuousSound LoseMusic_;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	TMap<TSoftObjectPtr<UWorld>, FMusicForLevel> MusicForLevels_;
};
