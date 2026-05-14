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

public:
	float StartTimeRandomized() const
	{
		return FMath::RandRange( StartTimeRange_.X, StartTimeRange_.Y );
	}

	float InitialFadeInRandomized() const
	{
		return FMath::RandRange( InitialFadeInRange_.X, InitialFadeInRange_.Y );
	}

	float FadeInRandomized() const
	{
		return FMath::RandRange( FadeInRange_.X, FadeInRange_.Y );
	}

	float FadeOutRandomized() const
	{
		return FMath::RandRange( FadeOutRange_.X, FadeOutRange_.Y );
	}

	float DelayBeforeStartRandomized() const
	{
		return FMath::RandRange( DelayBeforeStartRange_.X, DelayBeforeStartRange_.Y );
	}

	float RepeatDelayRandomized() const
	{
		return FMath::RandRange( RepeatDelayRange_.X, RepeatDelayRange_.Y );
	}

	UPROPERTY( EditAnywhere )
	TObjectPtr<USoundBase> Sound;

	UPROPERTY( EditAnywhere, meta = ( ClampMin = 0.0f, ClampMax = 1.0f ) )
	float Volume = 1.0f;

	UPROPERTY( EditAnywhere, meta = ( ClampMin = 0.01f ) )
	float Pitch = 1.0f;

protected:
	UPROPERTY( EditAnywhere, meta = ( ClampMin = 0.0f ) )
	FVector2D StartTimeRange_ = FVector2D::ZeroVector;

	UPROPERTY( EditAnywhere, meta = ( ClampMin = 0.0f ) )
	FVector2D DelayBeforeStartRange_ = FVector2D::ZeroVector;

	UPROPERTY( EditAnywhere, meta = ( ClampMin = 0.0f ) )
	FVector2D InitialFadeInRange_ = FVector2D::ZeroVector;

	UPROPERTY( EditAnywhere, meta = ( ClampMin = 0.0f ) )
	FVector2D FadeInRange_ = FVector2D::ZeroVector;

	UPROPERTY( EditAnywhere, meta = ( ClampMin = 0.0f ) )
	FVector2D FadeOutRange_ = FVector2D::ZeroVector;

	UPROPERTY( EditAnywhere, meta = ( ClampMin = 0.0f, Tooltip = "Sound will play again after specified time" ) )
	FVector2D RepeatDelayRange_ = FVector2D::ZeroVector;
};
