#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Lords_Frontiers/Public/Resources/GameResource.h"  

#include "EconomyComponent.generated.h"

class ABuilding;
class AGridManager;
class UResourceManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnEconomyBalanceChanged, const FResourceProduction&, NetIncome );

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API UEconomyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEconomyComponent();
	// scan grid and accrual resource
	// button in UI
	UFUNCTION( BlueprintCallable, Category = "Settings|Economy" )
	void CollectGlobalResources();

	UFUNCTION( BlueprintCallable, Category = "Settings|Economy" )
	void ApplyMaintenanceCosts();

	void RegisterBuilding( class ABuilding* building );
	void UnregisterBuilding( class ABuilding* building );

	void SetResourceManager( UResourceManager* InManager )
	{
		ResourceManager_ = InManager;
	}

	void PerformInitialScan();

	void RestoreAllBuildings();

	UFUNCTION( BlueprintPure, Category = "Settings|Economy|Balance" )
	FResourceProduction GetTotalProduction();

	UFUNCTION( BlueprintPure, Category = "Settings|Economy|Balance" )
	FResourceProduction GetTotalMaintenance();

	UFUNCTION( BlueprintPure, Category = "Settings|Economy|Balance" )
	FResourceProduction GetNetIncome();

	UFUNCTION( BlueprintPure, Category = "Settings|Economy|Balance" )
	int32 GetNetIncomeForType( EResourceType Type );

	UFUNCTION( BlueprintCallable, Category = "Settings|Economy|Balance" )
	void MarkEconomyDirty();

	UPROPERTY( BlueprintAssignable, Category = "Settings|Economy|Events" )
	FOnEconomyBalanceChanged OnEconomyBalanceChanged;

protected:
	virtual void BeginPlay() override;


private:
	void RecalculateIfDirty();
	void RecalculateEconomyBalance();

	FResourceProduction CachedTotalProduction_;
	FResourceProduction CachedTotalMaintenance_;
	FResourceProduction CachedNetIncome_;

	bool bEconomyDirty_ = true;
	// url system
	UPROPERTY()
	AGridManager* GridManager_;

	UPROPERTY()
	UResourceManager* ResourceManager_;

	UPROPERTY()
	TArray<TWeakObjectPtr<class ABuilding>> RegisteredBuildings_;

	UPROPERTY()
	bool bInitialScanDone = false;

	// dependency search
	void FindSystems();
};
