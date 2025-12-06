#pragma once

#include "CoreMinimal.h"
#include "Building.h"
#include "ResourceBuilding.generated.h"

// Abstract base class for buildings that generate resources.
UCLASS(Abstract)
class LORDS_FRONTIERS_API AResourceBuilding : public ABuilding
{
	GENERATED_BODY()

public:
	AResourceBuilding();
};