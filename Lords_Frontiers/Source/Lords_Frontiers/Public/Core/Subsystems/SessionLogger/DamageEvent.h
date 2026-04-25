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

DECLARE_MULTICAST_DELEGATE_TwoParams( FOnProjectileMissed, AActor*, const FVector& );

/**
 * Fired when a projectile reaches its target location (FlightProgress = 1.0),
 * regardless of whether it hit an enemy or impacted on empty ground.
 * Mid-flight collisions do NOT raise this event.
 */
DECLARE_MULTICAST_DELEGATE_TwoParams( FOnProjectileLanded, AActor*, const FVector& );

struct LORDS_FRONTIERS_API FDamageEvents
{
	static FOnDamageDealt OnDamageDealt;
	static FOnProjectileMissed OnProjectileMissed;
	static FOnProjectileLanded OnProjectileLanded;
};
