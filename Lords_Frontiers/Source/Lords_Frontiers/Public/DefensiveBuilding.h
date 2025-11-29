#pragma once

#include "CoreMinimal.h"
#include "Building.h"
#include "DefensiveBuilding.generated.h"

// Abstract base class for defensive towers and turrets.
UCLASS(Abstract)
class LORDS_FRONTIERS_API ADefensiveBuilding : public ABuilding
{
	GENERATED_BODY()

public:
	ADefensiveBuilding();
};