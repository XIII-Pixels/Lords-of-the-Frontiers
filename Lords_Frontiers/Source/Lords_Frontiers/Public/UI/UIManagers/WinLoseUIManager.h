// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UIManager.h"

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "WinLoseUIManager.generated.h"

class UWinLoseWidget;
/** (Gregory-hub)
 * Handles all UI/UX logic for win level and lose level
 * Should be part of GameMode
 * Configuration parameters are in corresponding GameMode */
UCLASS()
class LORDS_FRONTIERS_API UWinLoseUIManager : public UUIManager
{
	GENERATED_BODY()

public:
	virtual void SetupWidget( TSubclassOf<UUserWidget> widgetClass ) override;

protected:
	UPROPERTY()
	TObjectPtr<UWinLoseWidget> WinLoseWidget_;

	UFUNCTION()
	void OnMainMenuButtonClicked();

	UFUNCTION()
	void OnNewGameButtonClicked();

	UFUNCTION()
	void OnQuitButtonClicked();
};
