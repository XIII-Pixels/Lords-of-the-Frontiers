// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DebugGameMode.generated.h"


/** (Gregory-hub)
 * GameMode for debugging
 * Loads specified widget and shows mouse cursor
 */
UCLASS(HideDropdown)
class LORDS_FRONTIERS_API ADebugGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = Debug)
	TSubclassOf<UUserWidget> DebugHUDClass;

	virtual void PostInitProperties() override;

protected:
	UPROPERTY()
	TObjectPtr<UUserWidget> DebugHUD_;

	virtual void BeginPlay() override;

	void ShowMouseCursor() const;
};
