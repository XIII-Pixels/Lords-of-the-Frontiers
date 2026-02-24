// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "TargetBuildingTracker.generated.h"

class AUnit;
class ABuilding;

USTRUCT(BlueprintType)
struct FBuildingSet
{
	GENERATED_BODY()

	TSet<TWeakObjectPtr<ABuilding>> Buildings;
};

/** (Gregory-hub)
 * Tracks target buildings that are on a level
 * On wave start: for each unit type finds all building objects that unit can attack
 * On building destruction: removes building from building list
 * Used to find next unit target */
UCLASS()
class LORDS_FRONTIERS_API UTargetBuildingTracker : public UObject
{
	GENERATED_BODY()

public:
	// Needs to be called when wave starts
	void OnWaveStart();

	// Needs to be called when any building is destroyed
	void OnBuildingDestroyed( ABuilding* building );

	void SetUnitClasses( const TSet<TSubclassOf<AUnit>>& unitClasses )
	{
		for ( auto unitClass : unitClasses )
		{
			TargetBuildings_.FindOrAdd( unitClass );
		}
	}

protected:
	// Scans level for all buildings that can be attacked by each unit types
	// Puts buildings into PriorityBuildings_
	void ScanLevelForBuildings();

	UPROPERTY()
	TMap<TSubclassOf<AUnit>, FBuildingSet> TargetBuildings_;
};
