// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AudioSettingsSubsystem.h"
#include "Data/LoopingSoundConfig.h"

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "LoopingSound.generated.h"

/** (Gregory-hub)
 * Sound that loops according to sound settings
 * IMPORTANT: sound timers use game time. Setting game speed makes them invalid */
UCLASS()
class LORDS_FRONTIERS_API ULoopingSound : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * @param extraVolumeScale - Additional multiplier on top of the config volume and the
	 *        category volume; used e.g. to play battle music quieter than its track volume.
	 */
	void Initialize(
	    const FLoopingSoundConfig* soundConfig, EAudioCategory category = EAudioCategory::Effects,
	    float extraVolumeScale = 1.0f );
	void Play();
	void Stop( bool instant = false );

	UAudioComponent* GetAudioComponent()
	{
		return AudioComponent_;
	}

	const FLoopingSoundConfig* GetConfig() const
	{
		return SoundConfig_;
	}

	EAudioCategory GetCategory() const
	{
		return Category_;
	}

protected:
	virtual void BeginDestroy() override;

	UPROPERTY()
	TObjectPtr<UAudioComponent> AudioComponent_;

	const FLoopingSoundConfig* SoundConfig_;

private:
	void StartPlayback( bool initial = false );

	void ClearTimers();

	UFUNCTION()
	void HandleVolumeChanged( EAudioCategory changedCategory, float newVolume );

	void ApplyCategoryVolume();

	EAudioCategory Category_ = EAudioCategory::Effects;

	float ExtraVolumeScale_ = 1.0f;

	FTimerHandle ReplayTimerHandle_;
	FTimerHandle FadeTimerHandle_;
	FTimerHandle StartTimerHandle_;
};
