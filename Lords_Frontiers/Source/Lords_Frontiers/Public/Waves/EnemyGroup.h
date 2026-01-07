#pragma once

#include "CoreMinimal.h"
#include "Lords_Frontiers/Public/Units/Unit.h"
#include "EnemyGroup.generated.h"

/*
(Artyom)
Describes a single group of identical enemies to be spawned in a wave
 
- EnemyClass: which enemy type to spawn (derived from AUnit)
- Count: how many units to spawn
- SpawnInterval: delay between spawning individual units in the group (seconds)
if <= 0 — all units spawn instantly
 */
USTRUCT(BlueprintType)
struct FEnemyGroup
{
	GENERATED_BODY()

public:

	// Enemy type to spawn (class derived from AUnit)
	UPROPERTY ( EditAnywhere, BlueprintReadWrite, Category = "Spawn" )
	TSubclassOf <AUnit> EnemyClass = nullptr;

	// Number of units that should be spawned
	UPROPERTY ( EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = ( ClampMin = "0" ) )
	int32 Count = 0;

	/**
	 * Interval between spawning individual units in this group (seconds).
	 * If <= 0 — all units spawn instantly.
	 */
	UPROPERTY ( EditAnywhere, BlueprintReadWrite, Category = "Spawn" )
	float SpawnInterval = 0.0f;

public:

	FEnemyGroup ();
	FEnemyGroup ( TSubclassOf <AUnit> inClass, int32 inCount, float inSpawnInterval = 0.0f );

	// Returns true if EnemyClass is valid and Count > 0
	bool IsValid () const;

	// Returns true if units should spawn instantly
	bool IsInstantSpawn () const;

	// Returns delay before the spawn of the i-th unit (0-based)
	// If Index < 0 — returns 0. If IsInstantSpawn() — returns 0.
	float GetSpawnDelayForIndex ( int32 index ) const;

	// Total time required to spawn the whole group (seconds)
	// For Count <= 1 or instant spawn returns 0
	float GetTotalSpawnDuration () const;

	// Returns the class name of the enemy (or "None" if class not set)
	FString GetEnemyClassName () const;
};