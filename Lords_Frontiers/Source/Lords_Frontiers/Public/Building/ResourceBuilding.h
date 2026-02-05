#pragma once

#include "Building.h"
#include "CoreMinimal.h"
#include "Lords_Frontiers/Public/Resources/ResourceGenerator.h"

#include "ResourceBuilding.generated.h"

class UResourceManager;

// Abstract base class for buildings that generate resources.
UCLASS( Abstract )
class LORDS_FRONTIERS_API AResourceBuilding : public ABuilding
{
	GENERATED_BODY()

public:
	AResourceBuilding();

	UFUNCTION( BlueprintPure, Category = "Settings|Resource Generation" )
	UResourceGenerator* GetResourceGenerator() const
	{
		return ResourceGenerator_;
	}

	const FResourceProduction& GetProductionConfig() const
	{
		return ProductionConfig_;
	}

	/**
	 * Modifies ProductionConfig for a specific resource type.
	 * Updates ResourceGenerator to reflect the change.
	 * @param type - Which resource to modify
	 * @param delta - Value to add (positive = more production)
	 */
	void ModifyProduction( EResourceType type, int32 delta );

	/**
	 * Modifies production for all resource types at once.
	 * @param delta - Value to add to each resource
	 */
	void ModifyProductionAll( int32 delta );

	/**
	 * Resets ProductionConfig to original values from CDO.
	 * Call on game restart to undo all card modifications.
	 */
	void ResetProductionToDefaults();

protected:
	virtual void BeginPlay() override;

	/** Pushes current ProductionConfig_ to the ResourceGenerator */
	void SyncGeneratorConfig();

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Resource Generation" )
	FResourceProduction ProductionConfig_;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Resource Generation" )
	UResourceGenerator* ResourceGenerator_;

private:
	UResourceManager* FindResourceManager() const;

	/** Snapshot of ProductionConfig_ saved in BeginPlay for reset */
	FResourceProduction OriginalProductionConfig_;
};
