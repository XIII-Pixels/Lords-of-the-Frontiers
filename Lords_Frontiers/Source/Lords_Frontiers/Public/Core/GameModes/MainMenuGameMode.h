// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "MainMenuGameMode.generated.h"

class UMainMenuUIManager;

/** (Gregory-hub)
 * Game mode for main menu */
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API AMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void InitGame( const FString& mapName, const FString& options, FString& errorMessage ) override;

	virtual void StartPlay() override;

protected:
	UPROPERTY()
	TObjectPtr<UMainMenuUIManager> UIManager_;

	UPROPERTY( EditDefaultsOnly, Category = "Settings" )
	TSubclassOf<UUserWidget> MainMenuWidgetClass_;
};
