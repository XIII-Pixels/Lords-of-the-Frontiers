#pragma once

#include "Building.h"
#include "CoreMinimal.h"
#include "Lords_Frontiers/Public/ResourceManager/ResourceGenerator.h"

#include "ResourceBuilding.generated.h"

class UResourceManager;

// Abstract base class for buildings that generate resources.
UCLASS( Abstract )
class LORDS_FRONTIERS_API AResourceBuilding : public ABuilding
{
	GENERATED_BODY()

  public:
	AResourceBuilding();

	virtual EBuildingCategory GetBuildingCategory() const override;

  protected:
	virtual void BeginPlay() override;

  protected:
	// The component responsible for the core resource generation logic (type, quantity, interval).
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Resource Generation" )
	UResourceGenerator* ResourceGenerator_;

  private:
	// Find the Resource Manager
	UResourceManager* FindResourceManager_() const;
};