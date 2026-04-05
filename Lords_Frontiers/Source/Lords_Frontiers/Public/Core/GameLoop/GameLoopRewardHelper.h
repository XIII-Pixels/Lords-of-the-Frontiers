// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "GameLoopRewardHelper.generated.h"

class UGameLoopConfig;
class UResourceManager;
class UEconomyComponent;
struct FResourceReward;

/**
 *
 */
UCLASS()
class LORDS_FRONTIERS_API UGameLoopRewardHelper : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, Category = "GameLoop|Rewards" )
	void Initialize( UGameLoopConfig* config, UResourceManager* rm, UEconomyComponent* ec );

	UFUNCTION( BlueprintCallable, Category = "GameLoop|Rewards" )
	void GrantStartingResources();

	UFUNCTION( BlueprintCallable, Category = "GameLoop|Rewards" )
	void GrantCombatReward( int32 wave, bool bPerfectWave );

	UFUNCTION( BlueprintCallable, Category = "GameLoop|Rewards" )
	void CollectBuildingIncome();

	UFUNCTION( BlueprintCallable, Category = "GameLoop|Rewards" )
	void ApplyMaintenanceCosts();

	UFUNCTION( BlueprintCallable, Category = "GameLoop|Rewards" )
	void ApplyReward( FResourceReward& reward );

	UFUNCTION( BlueprintCallable, Category = "GameLoop|Rewards" )
	void RecalculateIncome();

	UFUNCTION( BlueprintCallable, Category = "GameLoop|Rewards" )
	void RestoreBuildings();

private:
	UPROPERTY()
	TObjectPtr<UGameLoopConfig> Config_;

	UPROPERTY()
	TWeakObjectPtr<UResourceManager> ResourceManager_;

	UPROPERTY()
	TWeakObjectPtr<UEconomyComponent> EconomyComponent_;
};
