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

private:
	//url system
	UPROPERTY()
	AGridManager* GridManager_;

	UPROPERTY()
	UResourceManager* ResourceManager_;

	//dependency search
	void FindSystems();
};