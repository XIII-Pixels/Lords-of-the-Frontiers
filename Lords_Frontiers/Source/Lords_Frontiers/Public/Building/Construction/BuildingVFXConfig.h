// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "BuildingVFXConfig.generated.h"

class UNiagaraSystem;

/**
 * Maxim
 */
UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UBuildingVFXConfig : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Construction" )
	TObjectPtr<UNiagaraSystem> ConstructionVFX = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Construction", meta = ( ClampMin = "0.0", Units = "s" ) )
	float ConstructionDelay = 0.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Destruction", meta = ( ClampMin = "0.0", Units = "s" ) )
	float RuinDelay = 0.0f;
};