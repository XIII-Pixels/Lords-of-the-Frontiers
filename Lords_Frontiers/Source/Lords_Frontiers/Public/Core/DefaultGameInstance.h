// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Subsystems/LevelSubsystem/LevelsDataAsset.h"

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "DefaultGameInstance.generated.h"

/** (Gregory-hub) */
UCLASS( Abstract )
class LORDS_FRONTIERS_API UDefaultGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

protected:
	UPROPERTY( EditDefaultsOnly, Category = "Settings|Levels" )
	TObjectPtr<ULevelsDataAsset> Levels_;
};
