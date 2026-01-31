#pragma once

#include "Building.h"
#include "Lords_Frontiers/Public/Resources/ResourceGenerator.h"

#include "CoreMinimal.h"

#include "ResourceBuilding.generated.h"

class UResourceManager;

// Abstract base class for buildings that generate resources.
UCLASS( Abstract )
class LORDS_FRONTIERS_API AResourceBuilding : public ABuilding
{
	GENERATED_BODY()

public:
	AResourceBuilding();

	// Accessor for the economy scanner
	UFUNCTION( BlueprintPure, Category = "Settings|Resource Generation" )
	UResourceGenerator* GetResourceGenerator() const
	{
		return ResourceGenerator_;
	}

protected:
	virtual void BeginPlay() override;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Resource Generation" )
	FResourceProduction ProductionConfig;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Resource Generation" )
	UResourceGenerator* ResourceGenerator_;

private:
	// Find the Resource Manager
	UResourceManager* FindResourceManager() const;
};
