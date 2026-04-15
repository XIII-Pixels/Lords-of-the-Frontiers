// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "UnitBuilder.generated.h"

struct FEnemyBuff;
struct FEntityStats;
class AUnit;

/** (Gregory-hub)
 * Class for creating and spawning units */
UCLASS()
class LORDS_FRONTIERS_API UUnitBuilder : public UObject
{
	GENERATED_BODY()

public:
	void CreateNewUnit(
	    const TSubclassOf<AUnit>& subclass, const FTransform& transform, AActor* owner = nullptr,
	    APawn* instigator = nullptr
	);
	void SetTransform( const FTransform& transform ) const;
	void SetStats( const FEntityStats& stats ) const;
	void ApplyBuff( const FEnemyBuff* buff ) const;
	TWeakObjectPtr<AUnit> SpawnUnitAndFinish();

	// (Artyom)
    // find location around enemy group spawnpoint if there is a unit
    FTransform FindNonOverlappingSpawnTransform(
        const FTransform& desiredTransform, float capsuleRadius, float capsuleHalfHeight, float maxSearchRadius,
        int32 maxAttempts, bool bProjectToNavMesh
    ) const;

private:
	UPROPERTY()
	TObjectPtr<AUnit> Unit_;
};
