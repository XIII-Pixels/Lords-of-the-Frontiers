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

protected:
	// Attack range (for the tower's component)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense")
	float AttackRange_;

	// Attack damage (can be taken from FEntityStats, but often duplicated)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense")
	int32 DefenseDamage_;
};