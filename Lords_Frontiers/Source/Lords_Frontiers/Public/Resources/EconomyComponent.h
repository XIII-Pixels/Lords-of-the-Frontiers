#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "EconomyComponent.generated.h"

class ABuilding;
class AGridManager;
class UResourceManager;

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API UEconomyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEconomyComponent();

protected:
	virtual void BeginPlay() override;

public:
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

private:
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
