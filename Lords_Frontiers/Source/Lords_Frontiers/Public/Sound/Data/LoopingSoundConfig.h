// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "LoopingSoundConfig.generated.h"

class USoundBase;

/** (Gregory-hub)
 * Looping sound */
USTRUCT( BlueprintType )
struct FLoopingSoundConfig
{
	GENERATED_BODY()

public:
	float StartTimeRandomized() const
	{
		return FMath::RandRange( StartTime_.X, StartTime_.Y );
	}

	float InitialFadeInRandomized() const
	{
		return FMath::RandRange( InitialFadeIn_.X, InitialFadeIn_.Y );
	}

	float FadeInRandomized() const
	{
		return FMath::RandRange( FadeIn_.X, FadeIn_.Y );
	}

	float FadeOutRandomized() const
	{
		return FMath::RandRange( FadeOut_.X, FadeOut_.Y );
	}

	float TransitionFadeOutRandomized() const
	{
		return FMath::RandRange( TransitionFadeOut_.X, TransitionFadeOut_.Y );
	}

	float DelayBeforeStartRandomized() const
	{
		return FMath::RandRange( DelayBeforeStart_.X, DelayBeforeStart_.Y );
	}

	float RepeatDelayRandomized() const
	{
		return FMath::RandRange( RepeatDelay_.X, RepeatDelay_.Y );
	}

	UPROPERTY( EditAnywhere )
	TObjectPtr<USoundBase> Sound;

	UPROPERTY( EditAnywhere, meta = ( ClampMin = 0.0f, ClampMax = 1.0f ) )
	float Volume = 1.0f;

	UPROPERTY( EditAnywhere, meta = ( ClampMin = 0.01f ) )
	float Pitch = 1.0f;

protected:
	UPROPERTY( EditAnywhere, meta = ( ClampMin = 0.0f ) )
	FVector2D StartTime_ = FVector2D::ZeroVector;

	UPROPERTY( EditAnywhere, meta = ( ClampMin = 0.0f ) )
	FVector2D DelayBeforeStart_ = FVector2D::ZeroVector;

	UPROPERTY( EditAnywhere, meta = ( ClampMin = 0.0f ) )
	FVector2D InitialFadeIn_ = FVector2D::ZeroVector;

	UPROPERTY( EditAnywhere, meta = ( ClampMin = 0.0f ) )
	FVector2D FadeIn_ = FVector2D::ZeroVector;

	UPROPERTY( EditAnywhere, meta = ( ClampMin = 0.0f ) )
	FVector2D FadeOut_ = FVector2D::ZeroVector;

	UPROPERTY( EditAnywhere, meta = ( ClampMin = 0.0f ) )
	FVector2D TransitionFadeOut_ = { 1.0f, 1.0f };

	UPROPERTY( EditAnywhere, meta = ( ClampMin = 0.0f, Tooltip = "Sound will play again after specified time" ) )
	FVector2D RepeatDelay_ = FVector2D::ZeroVector;
};
