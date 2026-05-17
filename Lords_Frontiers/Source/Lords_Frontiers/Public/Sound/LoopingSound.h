// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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
	void Initialize( const FLoopingSoundConfig* soundConfig );
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

protected:
	virtual void BeginDestroy() override;

	UPROPERTY()
	TObjectPtr<UAudioComponent> AudioComponent_;

	const FLoopingSoundConfig* SoundConfig_;

private:
	void StartPlayback( bool initial = false );

	void ClearTimers();

	FTimerHandle ReplayTimerHandle_;
	FTimerHandle FadeTimerHandle_;
	FTimerHandle StartTimerHandle_;
};
