// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UIOnlyGameMode.h"

#include "CoreMinimal.h"

#include "MainMenuGameMode.generated.h"

/** (Gregory-hub) */
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API AMainMenuGameMode : public AUIOnlyGameMode
{
	GENERATED_BODY()

public:
	virtual void StartPlay() override;
};
