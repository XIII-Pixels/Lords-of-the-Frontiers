// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "DebugGameMode.generated.h"

class UUserWidget;

UCLASS( HideDropdown )
class LORDS_FRONTIERS_API ADebugGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADebugGameMode();

	UPROPERTY( EditDefaultsOnly, Category = Debug )
	TSubclassOf<UUserWidget> DebugHUDClass;

protected:
	UPROPERTY()
	TObjectPtr<UUserWidget> DebugHUD_;

	virtual void BeginPlay() override;

	void SetupStaticCamera() const;
};
