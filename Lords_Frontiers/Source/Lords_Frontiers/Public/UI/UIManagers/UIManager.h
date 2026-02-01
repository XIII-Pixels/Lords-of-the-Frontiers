// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "UIManager.generated.h"

/** (Gregory-hub)
 * Base class for UI managers */
UCLASS( Abstract )
class LORDS_FRONTIERS_API UUIManager : public UObject
{
	GENERATED_BODY()

public:
	void OnStartPlay();

	virtual void SetupWidget( TSubclassOf<UUserWidget> widgetClass ) PURE_VIRTUAL();

protected:
	UPROPERTY()
	TObjectPtr<UUserWidget> Widget_;
};
