// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UIManager.h"

#include "CoreMinimal.h"

#include "MainMenuUIManager.generated.h"

class UMainMenuWidget;

/** (Gregory-hub)
 * Handles all UI/UX logic in main menu
 * Should be part of GameMode
 * Configuration parameters are in corresponding GameMode */
UCLASS()
class LORDS_FRONTIERS_API UMainMenuUIManager : public UUIManager
{
	GENERATED_BODY()

public:
	virtual void SetupWidget( TSubclassOf<UUserWidget> widgetClass ) override;

protected:
	UFUNCTION()
	void OnNewGameButtonClicked();
};
