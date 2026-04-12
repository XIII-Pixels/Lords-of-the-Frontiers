#pragma once

#include "CoreMinimal.h"

#include "ProjectileTypes.generated.h"

UENUM( BlueprintType )
enum class EProjectileType : uint8
{
	Arrow_Tower,
	Arrow_Enemy,
	Cannonball,
	MagicProjectile,
	Fireball_Enemy,
	Melee_Single,
	Melee_AoE,
	Catapult
};