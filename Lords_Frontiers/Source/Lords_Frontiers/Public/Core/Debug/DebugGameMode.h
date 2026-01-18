// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "DebugGameMode.generated.h"

class UUserWidget;

UCLASS( Abstract )
class LORDS_FRONTIERS_API ADebugGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADebugGameMode();

	UPROPERTY( EditDefaultsOnly, Category = "Settings" )
	TSubclassOf<UUserWidget> DebugHUDClass;

	virtual void StartPlay() override;

protected:
	UPROPERTY()
	TObjectPtr<UUserWidget> DebugHUD_;

	void SetupStaticCamera() const;
};
