#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ResourceTypes.h"
#include "ResourceManager.generated.h"

// Component responsible for storing and managing the resources of the owner (Player)
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LORDS_FRONTIERS_API UResourceManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UResourceManager();

protected:
	virtual void BeginPlay() override;

public:
	// Adds the specified amount of resource
	UFUNCTION(BlueprintCallable, Category = "Resource Management")
	void AddResource(EResourceType Type, int32 Quantity);

	// Trying to spend a resource. Returns true if there are enough resources and the debit was successful.
	UFUNCTION(BlueprintCallable, Category = "Resource Management")
	bool TrySpendResource(EResourceType Type, int32 Quantity);

	// Returns the current amount of the resource
	UFUNCTION(BlueprintPure, Category = "Resource Management")
	int32 GetResourceAmount(EResourceType Type) const;

	// Checks if the resource is sufficient without debiting it.
	UFUNCTION(BlueprintPure, Category = "Resource Management")
	bool HasEnoughResource(EResourceType Type, int32 Quantity) const;

private:
	// Resource Storage: Key - Type, Value - Quantity
	UPROPERTY(VisibleAnywhere, Category = "Resources")
	TMap<EResourceType, int32> Resources_;
};