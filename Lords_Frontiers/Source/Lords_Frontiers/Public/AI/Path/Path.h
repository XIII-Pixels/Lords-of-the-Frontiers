// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "DStarLite.h"

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "Path.generated.h"

class AGridManager;
class AUnit;

/** (Gregory-hub)
 * Class that represents a path that is traveled by unit */
UCLASS( NotBlueprintable )
class LORDS_FRONTIERS_API UPath : public UObject
{
	GENERATED_BODY()

public:
	UPath();

	virtual void PostInitProperties() override;

	// Setup

	// Grid is required to calculate path
	void SetGrid( TWeakObjectPtr<AGridManager> grid );

	// Unit damage and cooldown are needed to calculate time to destroy a building
	void SetUnitAttackInfo( float damage, float cooldown );

	void SetEmptyCellTravelTime( float emptyCellTravelTime );

	void SetStartAndGoal( const FIntPoint& start, const FIntPoint& goal );

	// Pathfinding

	// Update cell values. Path needs to be updated after calling this
	void OnUpdateCell( const FIntPoint& cell );
	// Calculate full path or update part of path (if grid has changed)
	void CalculateOrUpdate();

	const TArray<FIntPoint>& GetPoints() const;

private:
	UPROPERTY()
	TObjectPtr<UDStarLite> DStarLite_;

	UPROPERTY()
	TWeakObjectPtr<AGridManager> Grid_;

	bool bStartGoalInitialized = false;

	TArray<FIntPoint> PathPoints_;
};
