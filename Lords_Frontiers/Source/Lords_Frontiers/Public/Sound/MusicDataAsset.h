// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "MusicDataAsset.generated.h"

USTRUCT( BlueprintType )
struct FMusicForLevel
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere )
	TObjectPtr<USoundBase> Building;

	UPROPERTY( EditAnywhere )
	TObjectPtr<USoundBase> Battle;
};

/** (Gregory-hub)
 * Music config */
UCLASS()
class LORDS_FRONTIERS_API UMusicDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	USoundBase* MainMenuMusic() const
	{
		return MainMenuMusic_;
	}

	USoundBase* WinBattleMusic() const
	{
		return WinBattleMusic_;
	}

	USoundBase* WinGameMusic() const
	{
		return WinGameMusic_;
	}

	USoundBase* LoseMusic() const
	{
		return LoseMusic_;
	}

	const FMusicForLevel* MusicForLevel( TSoftObjectPtr<UWorld> level ) const;

protected:
	UPROPERTY( EditAnywhere, Category = "Settings" )
	USoundBase* MainMenuMusic_ = nullptr;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	USoundBase* WinBattleMusic_ = nullptr;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	USoundBase* WinGameMusic_ = nullptr;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	USoundBase* LoseMusic_ = nullptr;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	TMap<TSoftObjectPtr<UWorld>, FMusicForLevel> MusicForLevels_;
};
