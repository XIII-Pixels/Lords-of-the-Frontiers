#pragma once

#include "GameplayTagContainer.h"

#include "CoreMinimal.h"

#include "AudioEvent.generated.h"

USTRUCT( BlueprintType )
struct FAudioEvent
{
	GENERATED_BODY()

	UPROPERTY( BlueprintReadWrite )
	FGameplayTag SoundTag;

	// Zero vector is treated as 2D sound
	UPROPERTY( BlueprintReadWrite )
	FVector WorldLocation = FVector::ZeroVector;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnAudioEvent, FAudioEvent, Event );
