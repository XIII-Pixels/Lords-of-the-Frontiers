#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EconomyComponent.generated.h"

class AGridManager;
class UResourceManager;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LORDS_FRONTIERS_API UEconomyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEconomyComponent();

protected:
	virtual void BeginPlay() override;

public:
	//scan grid and accrual resource
	//button in UI
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void CollectGlobalResources();

	void RegisterBuilding(class AResourceBuilding* Building);
	void UnregisterBuilding(class AResourceBuilding* Building);

	void SetResourceManager(UResourceManager* InManager) { ResourceManager_ = InManager; }

private:
	//url system
	UPROPERTY()
	AGridManager* GridManager_;

	UPROPERTY()
	UResourceManager* ResourceManager_;

	UPROPERTY()
	TArray<TWeakObjectPtr<class AResourceBuilding>> RegisteredBuildings_;

	//dependency search
	void FindSystems();
};