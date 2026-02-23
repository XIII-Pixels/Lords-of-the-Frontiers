// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "TargetBuildingTracker.generated.h"

class ABuilding;

/** (Gregory-hub)
 * Searches for all building objects that unit types can attack on wave start.
 * Tracks target buildings that are on a level
 * Used to find next unit target */
UCLASS()
class LORDS_FRONTIERS_API UTargetBuildingTracker : public UObject
{
	GENERATED_BODY()

public:
	// Needs to be called when wave starts
	void OnWaveStart();

protected:
	// Scans level for all buildings that can be attacked by unit
	// Puts buildings into PriorityBuildings_
	void ScanLevelForBuildings();

	UPROPERTY()
	TMap<TSubclassOf<ABuilding>, TSet<TWeakObjectPtr<ABuilding>>> PriorityBuildings_;
};
