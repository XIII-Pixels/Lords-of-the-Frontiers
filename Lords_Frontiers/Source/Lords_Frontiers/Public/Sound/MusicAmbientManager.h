// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"

#include "Components/AudioComponent.h"
#include "CoreMinimal.h"

#include "MusicAmbientManager.generated.h"

struct FContinuousSound;
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

	void PlayMusic( const FContinuousSound* sound );

	void PlayAmbient( const FContinuousSound* sound );

private:
	UAudioComponent* CreateAndPlay( const FContinuousSound* sound ) const;

	void PlayContinuousSound( UAudioComponent* audioComponent, const FContinuousSound* sound ) const;

	void StartPlayback( UAudioComponent* audioComponent, const FContinuousSound* sound, bool initial = false ) const;

	// Only one music track can play at a time
	UPROPERTY()
	TObjectPtr<UAudioComponent> Music_ = nullptr;

	// Any number of ambient tracks can play simultaneously
	UPROPERTY()
	TSet<TObjectPtr<UAudioComponent>> AmbientSounds_;

	// Pointers to data

	UPROPERTY()
	TWeakObjectPtr<UMusicDataAsset> MusicDataAsset_;

	UPROPERTY()
	TWeakObjectPtr<UAmbientDataAsset> AmbientDataAsset_;

	UPROPERTY()
	TWeakObjectPtr<ULevelsDataAsset> LevelsDataAsset_;
};
