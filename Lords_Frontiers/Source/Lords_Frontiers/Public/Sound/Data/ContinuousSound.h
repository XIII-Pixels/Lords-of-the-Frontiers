// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ContinuousSound.generated.h"

class USoundBase;

/** (Gregory-hub)
 * Looping sound */
USTRUCT( BlueprintType )
struct FContinuousSound
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere )
	TObjectPtr<USoundBase> Sound;

	UPROPERTY( EditAnywhere, meta = ( ClampMin = 0.0f ) )
	float Volume = 1.0f;

	UPROPERTY( EditAnywhere, meta = ( ClampMin = 0.01f ) )
	float PitchMult = 1.0f;

	UPROPERTY( EditAnywhere, meta = ( ClampMin = 0.0f ) )
	float FadeIn = 0.0f;

	UPROPERTY( EditAnywhere, meta = ( ClampMin = 0.0f ) )
	float FadeOut = 0.0f;

	UPROPERTY( EditAnywhere, meta = ( ClampMin = 0.0f ) )
	float DelayBeforeStart = 0.0f;

	UPROPERTY( EditAnywhere, meta = ( ClampMin = 0.0f, Tooltip = "Sound will play again after specified time" ) )
	float RepeatDelay = 0.0f;
};
