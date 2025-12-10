#include "Lords_Frontiers/Public/Building/DefensiveBuilding.h"

//Const for default defensive building
namespace
{
	constexpr int32 cDefaultDefenseDamage = 15;
	constexpr float cDefaultAttackRange = 500.0f;
}

ADefensiveBuilding::ADefensiveBuilding()
{
	// Initialization for defensive structures

	// Default stats: 100 Health, 0 Damage (override by DefenseDamage_)

	// Specific defensive properties
	DefenseDamage_ = cDefaultDefenseDamage;
	AttackRange_ = cDefaultAttackRange;
}