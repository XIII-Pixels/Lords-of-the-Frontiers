// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "AudioEventSource.generated.h"

class FOnAudioEvent;

UINTERFACE( MinimalAPI )
class UAudioEventSource : public UInterface
{
	GENERATED_BODY()
};

class IAudioEventSource
{
	GENERATED_BODY()
public:
	virtual FOnAudioEvent& GetOnAudioEvent() = 0;
};
