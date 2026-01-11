// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "MainMenuWidget.generated.h"

class UButton;

/** (Gregory-hub)
 * Main menu */
UCLASS( Abstract )
class LORDS_FRONTIERS_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UButton> NewGameButton;

protected:
	UFUNCTION()
	void OnNewGameButtonClicked();
};
