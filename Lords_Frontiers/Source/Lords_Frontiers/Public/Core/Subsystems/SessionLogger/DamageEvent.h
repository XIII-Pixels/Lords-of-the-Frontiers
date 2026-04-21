#pragma once

#include "CoreMinimal.h"

/**
 * Global delegate for tracking actual damage dealt.
 *
 * Fired at the exact point where TakeDamage() is called on targets,
 * so only actual damage is counted (misses, expired projectiles are excluded).
 *
 * Params:
 *   Instigator - The tower/unit that owns the attack (GetOwner() of projectile/component)
 *   Target     - The entity receiving damage
 *   Damage     - Actual damage amount applied
 *   bIsSplash  - True if this is area-of-effect (splash) damage
 */
DECLARE_MULTICAST_DELEGATE_FourParams( FOnDamageDealt, AActor*, AActor*, int, bool );

DECLARE_MULTICAST_DELEGATE_OneParam( FOnProjectileMissed, AActor* );

struct LORDS_FRONTIERS_API FDamageEvents
{
	static FOnDamageDealt OnDamageDealt;
	static FOnProjectileMissed OnProjectileMissed;
};
