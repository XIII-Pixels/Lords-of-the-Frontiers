// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Card.h"
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UpgradeManager.generated.h"

class UUpgradePreset;
class UCard;
/**
 * 
 */
UCLASS()
class LORDS_FRONTIERS_API UUpgradeManager : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	UFUNCTION()
	void AddCard(UCard* card);

	FCardModifiers GetModifiersForCategory(EBuildingCategory category);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting | Card | DebugPresetCard")
	UUpgradePreset* DebugPresetCard;

	UPROPERTY(EditFixedSize, BlueprintReadOnly, Category = "Setting | Card | ActiveCard")
	TArray<UCard*> ActiveCards;

private:


};
