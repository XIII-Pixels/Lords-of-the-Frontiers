// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UI/Widgets/MainMenuButtonWidget.h"
#include "UIManager.h"

#include "CoreMinimal.h"
#include "sound/AudioEvent.h"
#include "sound/AudioEventSource.h"

#include "MainMenuUIManager.generated.h"

class UMainMenuWidget;

/** Handles all UI/UX logic in main menu
 * Should be part of GameMode
 * Configuration parameters are in corresponding GameMode */
UCLASS()
class LORDS_FRONTIERS_API UMainMenuUIManager : public UUIManager, public IAudioEventSource
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
	void OnMenuActionRequested( EMainMenuButtonAction action );

	UFUNCTION()
	void OnMenuButtonHovered( EMainMenuButtonAction action );

	FOnAudioEvent OnAudioEvent_;
};
