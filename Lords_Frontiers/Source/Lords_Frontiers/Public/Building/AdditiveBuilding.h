#pragma once

#include "Building.h"

#include "CoreMinimal.h"

#include "AdditiveBuilding.generated.h"

// Abstract base class for utility buildings (walls, buffers, etc.)
UCLASS( Abstract )
class LORDS_FRONTIERS_API AAdditiveBuilding : public ABuilding
{
	GENERATED_BODY()

public:
	AAdditiveBuilding();

protected:
	// Example of a bonus applied by the building
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Utility" )
	float UtilityBonusValue_;
};