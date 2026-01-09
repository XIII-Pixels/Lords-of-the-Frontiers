#include "Lords_Frontiers/Public/Building/AdditiveBuilding.h"

// Const for default additive building
namespace
{
	constexpr float cDefaultUtilityBonusValue = 5.0f; // Example: 5% bonus or 5 health/sec
}

AAdditiveBuilding::AAdditiveBuilding()
{
	// Initialization for additive structures

	// Default stats from ABuilding

	// Specific utility property
	UtilityBonusValue_ = cDefaultUtilityBonusValue;
}
