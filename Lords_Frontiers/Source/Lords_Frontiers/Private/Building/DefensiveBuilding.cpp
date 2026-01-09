// DefensiveBuilding.cpp

#include "Building/DefensiveBuilding.h"

namespace
{
	constexpr int32 cDefaultDefenseDamage = 15;
	constexpr float cDefaultAttackRange = 500.0f;
} // namespace

ADefensiveBuilding::ADefensiveBuilding()
{
	PrimaryActorTick.bCanEverTick = false;

	DefenseDamage_ = cDefaultDefenseDamage;
	AttackRange_ = cDefaultAttackRange;
}

void ADefensiveBuilding::BeginPlay()
{
	Super::BeginPlay();
}
FString ADefensiveBuilding::GetNameBuild()
{
	return TEXT( "DefensiveBuild" );
}

EBuildingCategory ADefensiveBuilding::GetBuildingCategory() const
{
	return EBuildingCategory::Defensive;
}