// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "AnimationConfig.generated.h"

/** (Gregory-hub)
 * Contains animation with its data */
USTRUCT()
struct FAnimationConfig
{
	GENERATED_BODY()

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Animation", meta = ( ClampMin = 0.0f ) )
	TObjectPtr<UAnimSequence> Animation;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Animation", meta = ( ClampMin = 0.0f ) )
	float PlayRate = 1.0f;
};
