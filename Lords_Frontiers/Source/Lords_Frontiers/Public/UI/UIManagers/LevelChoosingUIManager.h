// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UIManager.h"

#include "CoreMinimal.h"
#include "sound/AudioEvent.h"
#include "sound/AudioEventSource.h"

#include "LevelChoosingUIManager.generated.h"

/** (Gregory-hub) */
UCLASS()
class LORDS_FRONTIERS_API ULevelChoosingUIManager : public UUIManager, public IAudioEventSource
{
	GENERATED_BODY()

public:
	virtual void SetupWidget( TSubclassOf<UUserWidget> widgetClass ) override;

	virtual void PostInitProperties() override;

	virtual void BeginDestroy() override;

	virtual FOnAudioEvent& GetOnAudioEvent() override
	{
		return OnAudioEvent_;
	}

protected:
	UFUNCTION()
	void OnLevelButtonClicked( int levelIndex );

	UFUNCTION()
	void OnLevelButtonHovered();

	UFUNCTION()
	void OnBackButtonClicked();

	UFUNCTION()
	void OnBackButtonHovered();

	FOnAudioEvent OnAudioEvent_;
};
