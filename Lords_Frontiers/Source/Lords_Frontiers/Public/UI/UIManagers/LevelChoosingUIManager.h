// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UIManager.h"

#include "CoreMinimal.h"

#include "LevelChoosingUIManager.generated.h"

/** (Gregory-hub) */
UCLASS()
class LORDS_FRONTIERS_API ULevelChoosingUIManager : public UUIManager
{
	GENERATED_BODY()

public:
	virtual void SetupWidget( TSubclassOf<UUserWidget> widgetClass ) override;

protected:
	UFUNCTION()
	void OnLevelButtonClicked( int levelIndex );

	UFUNCTION()
	void OnBackButtonClicked();
};
