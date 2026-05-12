// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"

#include "Components/AudioComponent.h"
#include "CoreMinimal.h"

#include "MusicAmbientManager.generated.h"

class ULevelsDataAsset;
class UAmbientDataAsset;
class UMusicDataAsset;

/** (Gregory-hub)
 * Manager for music and ambient */
UCLASS()
class LORDS_FRONTIERS_API UMusicAmbientManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void PlayMainMenuMusic();
	void PlayWinBattleMusic();
	void PlayWinGameMusic();
	void PlayLoseGameMusic();
	void PlayCurrentLevelBuildingMusic();
	void PlayCurrentLevelCombatMusic();

	void PlayCurrentLevelAmbient();

	void StopMusic();
	void StopAllAmbient();

	UAudioComponent* GetMusicPlaying() const
	{
		return Music_;
	}

	TSet<TObjectPtr<UAudioComponent>> GetAmbientSoundsPlaying() const
	{
		return AmbientSounds_;
	}

protected:
	virtual void Initialize( FSubsystemCollectionBase& collection ) override;
	virtual void Deinitialize() override;

	void PlayMusic( USoundBase* sound );

	void PlayAmbient( USoundBase* sound );

	UAudioComponent* PlayLoopingSound( USoundBase* sound ) const;

private:
	// Only one track can play at a time
	UPROPERTY()
	TObjectPtr<UAudioComponent> Music_ = nullptr;

	UPROPERTY()
	TSet<TObjectPtr<UAudioComponent>> AmbientSounds_;

	UPROPERTY()
	TWeakObjectPtr<UMusicDataAsset> MusicDataAsset_;

	UPROPERTY()
	TWeakObjectPtr<UAmbientDataAsset> AmbientDataAsset_;

	UPROPERTY()
	TWeakObjectPtr<ULevelsDataAsset> LevelsDataAsset_;
};
