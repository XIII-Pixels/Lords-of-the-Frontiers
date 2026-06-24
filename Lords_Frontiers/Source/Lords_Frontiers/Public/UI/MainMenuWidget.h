// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UI/Widgets/MainMenuButtonWidget.h"

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "MainMenuWidget.generated.h"

class UAudioSettingsWidget;

/** Main menu */
UCLASS( Abstract )
class LORDS_FRONTIERS_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY( BlueprintAssignable, Category = "MainMenu" )
	FOnMainMenuButtonClicked OnActionRequested;

	UPROPERTY( BlueprintAssignable, Category = "MainMenu" )
	FOnMainMenuButtonHovered OnButtonHovered;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Audio" )
	TSubclassOf<UAudioSettingsWidget> AudioSettingsWidgetClass;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleButtonClicked( EMainMenuButtonAction action );

	UFUNCTION()
	void HandleButtonHovered( EMainMenuButtonAction action );

	void OpenAudioSettings();

	UFUNCTION()
	void OnAudioSettingsClosed();

	UPROPERTY()
	TArray<TObjectPtr<UMainMenuButtonWidget>> MenuButtons_;

	UPROPERTY()
	TObjectPtr<UAudioSettingsWidget> ActiveAudioSettings_;
};
