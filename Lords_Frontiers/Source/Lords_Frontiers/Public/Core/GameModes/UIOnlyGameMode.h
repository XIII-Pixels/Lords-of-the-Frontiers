// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "UIOnlyGameMode.generated.h"

class UUIManager;
class UMainMenuUIManager;

/** (Gregory-hub)
 * Game mode for menus */
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API AUIOnlyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void InitGame( const FString& mapName, const FString& options, FString& errorMessage ) override;

	virtual void StartPlay() override;

protected:
	UPROPERTY( EditDefaultsOnly, Category = "Settings" )
	TSubclassOf<UUIManager> UIManagerClass_;

	UPROPERTY( EditDefaultsOnly, Category = "Settings" )
	TSubclassOf<UUserWidget> WidgetClass_;

	UPROPERTY()
	TObjectPtr<UUIManager> UIManager_;
};
