// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "MainMenuUIManager.generated.h"

class UMainMenuWidget;

/** (Gregory-hub)
 * Handles all UI/UX logic in main menu
 * Should be part of GameMode
 * Configuration parameters are in corresponding GameMode */
UCLASS()
class LORDS_FRONTIERS_API UMainMenuUIManager : public UObject
{
	GENERATED_BODY()

public:
	void OnStartPlay();

	void SetupMainMenuWidget(TSubclassOf<UMainMenuWidget> mainMenuWidgetClass);

protected:
	UPROPERTY()
	TObjectPtr<UMainMenuWidget> MainMenuWidget_;

	UFUNCTION()
	void OnNewGameButtonClicked();
};
