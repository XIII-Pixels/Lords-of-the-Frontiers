#pragma once

#include "GameResource.h"

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "ResourceManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnResourceChanged, EResourceType, Type, int32, NewAmount );

static constexpr int32 cDefaultMaxResource = 100;

// Component responsible for storing and managing the resources of the owner
// (Player)
UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API UResourceManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UResourceManager();

protected:
	virtual void BeginPlay() override;

public:
	// Adds the specified amount of resource
	UFUNCTION( BlueprintCallable, Category = "Resource Management" )
	void AddResource( EResourceType type, int32 quantity );

	// Trying to spend a resource. Returns true if there are enough resources and
	// the debit was successful.
	UFUNCTION( BlueprintCallable, Category = "Resource Management" )
	bool TrySpendResource( EResourceType type, int32 quantity );

	// Returns the current amount of the resource
	UFUNCTION( BlueprintPure, Category = "Resource Management" )
	int32 GetResourceAmount( EResourceType type ) const;

	// Checks if the resource is sufficient without debiting it.
	UFUNCTION( BlueprintPure, Category = "Resource Management" )
	bool HasEnoughResource( EResourceType type, int32 quantity ) const;

	UPROPERTY( BlueprintAssignable, Category = "Resource Management|Events" )
	FOnResourceChanged OnResourceChanged;

	int32 GetMaxResourceAmount( EResourceType type ) const;

private:
	// Resource Storage: Key - Type, Value - Quantity
	UPROPERTY( VisibleAnywhere, Category = "Resources" )
	TMap<EResourceType, int32> Resources_;

	UPROPERTY( EditAnywhere, Category = "Resources" )
	TMap<EResourceType, int32> MaxResources_;
};