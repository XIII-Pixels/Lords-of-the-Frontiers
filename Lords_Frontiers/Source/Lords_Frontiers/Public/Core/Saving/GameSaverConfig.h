// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "GameSaverConfig.generated.h"

/** (Gregory-hub) */
UCLASS()
class LORDS_FRONTIERS_API UGameSaverConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditDefaultsOnly, Category = "Settings", meta = ( ToolTip = "Disabled in shipping mode" ) )
	bool bClearAllSaveDataOnGameStart = false;
};
