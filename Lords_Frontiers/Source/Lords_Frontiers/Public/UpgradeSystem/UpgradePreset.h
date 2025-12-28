// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Card.h"
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UpgradePreset.generated.h"

/**
 * 
 */
UCLASS()
class LORDS_FRONTIERS_API UUpgradePreset : public UDataAsset
{
	GENERATED_BODY()
	

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Card|CardPreset")
	TArray <UCard*> CardPreset;
};
