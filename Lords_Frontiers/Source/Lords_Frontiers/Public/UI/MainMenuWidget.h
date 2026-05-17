// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "MainMenuWidget.generated.h"

class UButton;
class UAudioSettingsWidget;

/** (Gregory-hub)
 * Main menu */
UCLASS( Abstract )
class LORDS_FRONTIERS_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UButton> NewGameButton;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UButton> ExitGameButton;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> SettingsButton;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Audio" )
	TSubclassOf<UAudioSettingsWidget> AudioSettingsWidgetClass;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnSettingsClicked();

	UFUNCTION()
	void OnAudioSettingsClosed();

	UPROPERTY()
	TObjectPtr<UAudioSettingsWidget> ActiveAudioSettings_;
};
