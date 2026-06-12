// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"

#include "CoreMinimal.h"

#include "MusicAmbientManager.generated.h"

class ULoopingSound;
struct FLoopingSoundConfig;
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
	// Music
	void PlayMainMenuMusic();
	void PlayWinGameMusic();
	void PlayLoseGameMusic();
	void PlayCurrentLevelBuildingMusic();
	void PlayCurrentLevelCombatMusic();
	void StopMusic( bool instant = false );

	// Ambient
	void PlayCurrentLevelAmbient();
	void PlayWindAmbient();
	void StopWindAmbient();
	void StopAmbient( ULoopingSound* ambient );
	void StopAllAmbient( bool instant = false );

	void AdjustAmbientVolume( float volumeLevel );

	ULoopingSound* GetMusicPlaying() const
	{
		return Music_;
	}

	TSet<TObjectPtr<ULoopingSound>> GetAmbientSoundsPlaying() const
	{
		return AmbientSounds_;
	}

protected:
	virtual void Initialize( FSubsystemCollectionBase& collection ) override;
	virtual void Deinitialize() override;

	ULoopingSound* PlayMusic( const FLoopingSoundConfig* sound, float volumeScale = 1.0f );
	ULoopingSound* PlayAmbient( const FLoopingSoundConfig* sound );

private:
	enum class EMusicAmbientKind : uint8
	{
		Music,
		Ambient,
	};

	ULoopingSound* CreateAndPlay( const FLoopingSoundConfig* sound, EMusicAmbientKind kind, float volumeScale = 1.0f );

	// Only one music track can play at a time
	UPROPERTY()
	TObjectPtr<ULoopingSound> Music_ = nullptr;

	// Any number of ambient tracks can play simultaneously
	UPROPERTY()
	TSet<TObjectPtr<ULoopingSound>> AmbientSounds_;

	UPROPERTY()
	TWeakObjectPtr<ULoopingSound> WindAmbientPlaying_;

	// Pointers to data

	UPROPERTY()
	TWeakObjectPtr<UMusicDataAsset> MusicDataAsset_;

	UPROPERTY()
	TWeakObjectPtr<UAmbientDataAsset> AmbientDataAsset_;

	UPROPERTY()
	TWeakObjectPtr<ULevelsDataAsset> LevelsDataAsset_;
};
