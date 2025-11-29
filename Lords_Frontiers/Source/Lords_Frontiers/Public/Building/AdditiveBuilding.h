#pragma once

#include "CoreMinimal.h"
#include "Building.h"
#include "AdditiveBuilding.generated.h"

// Abstract base class for utility buildings (walls, buffers, etc.)
UCLASS(Abstract)
class LORDS_FRONTIERS_API AAdditiveBuilding : public ABuilding
{
	GENERATED_BODY()

public:
	AAdditiveBuilding();

protected:
	// Example of a bonus applied by the building
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility")
	float UtilityBonusValue_;
};