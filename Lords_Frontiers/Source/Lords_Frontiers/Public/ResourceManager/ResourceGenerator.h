#pragma once

#include "GameResource.h"

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "ResourceGenerator.generated.h"

class UResourceManager;

// Can be stored within GameState, PlayerController, or ResourceManager itself.
UCLASS( BlueprintType, Blueprintable )
class LORDS_FRONTIERS_API UResourceGenerator : public UObject
{
	GENERATED_BODY()

public:
	UResourceGenerator();

	// Explicit initialization to link with manager (Dependency Injection)
	UFUNCTION( BlueprintCallable, Category = "Initialization" )
	void Initialize( UResourceManager* manager );

	// Manual generation call (e.g., for "After Wave" logic)
	UFUNCTION( BlueprintCallable, Category = "Generation" )
	void GenerateNow();

	// Getters for the scanner
	UFUNCTION( BlueprintPure, Category = "Settings" )
	EResourceType GetResourceType() const
	{
		return ResourceType_;
	}

	UFUNCTION( BlueprintPure, Category = "Settings" )
	int32 GetGenerationQuantity() const
	{
		return GenerationQuantity_;
	}

protected:
	// Type of resource being generated
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings" )
	EResourceType ResourceType_;

	// Number of resources per action
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = ( ClampMin = "1" ) )
	int32 GenerationQuantity_;

private:
	// Internal generation logic
	void ProcessGeneration();

private:
	// ensuring GC handles references.
	UPROPERTY()
	UResourceManager* ResourceManager_;
};