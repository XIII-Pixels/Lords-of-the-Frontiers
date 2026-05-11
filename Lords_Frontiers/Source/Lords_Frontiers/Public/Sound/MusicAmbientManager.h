// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"

#include "Components/AudioComponent.h"
#include "CoreMinimal.h"

#include "MusicAmbientManager.generated.h"

USTRUCT( BlueprintType )
struct FMusicConfig
{
	GENERATED_BODY()

	/** The sound asset to play */
	UPROPERTY( EditAnywhere )
	USoundBase* Sound = nullptr;

	/** If true, this sound plays independently:
	 * - Will NOT stop other sounds already playing
	 * - Will NOT be stopped when other sounds begin playing */
	UPROPERTY( EditAnywhere )
	bool bIndependent = false;
};

/** (Gregory-hub)
 * Manager for music and ambient */
UCLASS()
class LORDS_FRONTIERS_API UMusicAmbientManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void PlayMainMenuMusic();
	void PlayBuildingMusic();
	void PlayCombatMusic();
	void PlayWinGameMusic();
	void PlayLoseGameMusic();

	void StopMusic();
	void StopAllAmbient();
	void StopAudio( UAudioComponent* audio );

	UAudioComponent* GetMusicPlaying() const
	{
		return Music_;
	}

	TSet<TObjectPtr<UAudioComponent>> GetAmbientSoundsPlaying() const
	{
		return AmbientSounds_;
	}

protected:
	void Play( const FMusicConfig& config );

	virtual void Deinitialize() override;

private:
	UPROPERTY()
	TObjectPtr<UAudioComponent> Music_ = nullptr;

	UPROPERTY()
	TSet<TObjectPtr<UAudioComponent>> AmbientSounds_;
};
