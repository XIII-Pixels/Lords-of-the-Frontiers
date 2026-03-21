// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Resources/GameResource.h"

#include "CoreMinimal.h"

#include "ResourceCollectionAnimData.generated.h"

/**
 * Maxim
 */
USTRUCT( BlueprintType )
struct FResourceCollectionEntry
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	EResourceType ResourceType = EResourceType::None;

	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	int32 Amount = 0;
};

/**
 * Maxim
 */
USTRUCT( BlueprintType )
struct FBuildingCollectionAnimData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FResourceCollectionEntry> BaseIncome;

	UPROPERTY()
	TArray<FResourceCollectionEntry> BonusIncome;

	UPROPERTY()
	bool bIsRuined = false;

	bool IsEmpty() const
	{
		return BaseIncome.Num() == 0 && BonusIncome.Num() == 0;
	}
};