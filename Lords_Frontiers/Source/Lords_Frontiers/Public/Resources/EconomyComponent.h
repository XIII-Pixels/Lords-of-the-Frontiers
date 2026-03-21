#pragma once

#include "Building/Animation/ResourcePopupAnimTypes.h"
#include "Resources/GameResource.h"

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "EconomyComponent.generated.h"

class ABuilding;
class AGridManager;
class UResourceManager;
class UCardDataAsset;
struct FEconomyBonuses;
class UResourceAnimConfig;
class UBonusIconsData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnNetIncomeChanged, const FResourceProduction&, NetIncome );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnCollectionAnimRequested, const TArray<FResourcePopupBatchEntry>&, Batch
);

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API UEconomyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEconomyComponent();

	UFUNCTION( BlueprintCallable, Category = "Settings|Economy" )
	void ResetEconomy();

	UPROPERTY( BlueprintAssignable, Category = "Settings|Economy|Events" )
	FOnNetIncomeChanged OnNetIncomeChanged;

	UPROPERTY( BlueprintAssignable, Category = "Settings|Economy|Events" )
	FOnCollectionAnimRequested OnCollectionAnimRequested;

	UFUNCTION( BlueprintPure, Category = "Settings|Economy" )
	FResourceProduction CalculateNetIncome() const;

	void RecalculateAndBroadcastNetIncome();

	// scan grid and accrual resource
	// button in UI
	UFUNCTION( BlueprintCallable, Category = "Settings|Economy" )
	void CollectGlobalResources();

	UFUNCTION( BlueprintCallable, Category = "Settings|Economy" )
	void ApplyMaintenanceCosts();

	void RegisterBuilding( class ABuilding* building );
	void UnregisterBuilding( class ABuilding* building );

	void SetResourceManager( UResourceManager* inManager )
	{
		ResourceManager_ = inManager;
	}

	void PerformInitialScan();

	void RestoreAllBuildings();

	void TriggerCollectionAnimations();

	UPROPERTY( EditAnywhere, Category = "Settings|Economy|Animation" )
	TObjectPtr<UResourceAnimConfig> ResourceAnimConfig_;

	UPROPERTY( EditAnywhere, Category = "Settings|Economy|Animation" )
	TObjectPtr<UBonusIconsData> BonusIconsData_;

protected:
	virtual void BeginPlay() override;

private:
	FResourceProduction CalculateTotalIncome() const;

	FResourceProduction CalculateTotalMaintenance() const;

	// url system
	UPROPERTY()
	AGridManager* GridManager_;

	UPROPERTY()
	UResourceManager* ResourceManager_;

	UPROPERTY()
	TArray<TWeakObjectPtr<class ABuilding>> RegisteredBuildings_;

	UPROPERTY()
	bool bInitialScanDone = false;

	FResourceProduction CachedNetIncome_;

	// dependency search
	void FindSystems();

	void SubscribeToCardEvents();

	UFUNCTION()
	void HandleCardsApplied( const TArray<UCardDataAsset*>& appliedCards );

	UFUNCTION()
	void HandleEconomyBonusesChanged( const FEconomyBonuses& newBonuses );
};
