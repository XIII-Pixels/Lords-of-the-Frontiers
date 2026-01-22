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

	// Grid is required to calculate path
	void SetGrid( TWeakObjectPtr<AGridManager> grid );

	// Find grid in the world. Fails if more than one grid is found
	void AutoSetGrid();

	// Set goal that must be reached
	void SetStartAndGoal( const FIntPoint& start, const FIntPoint& goal );

	// Update cell values. Path needs to be updated after calling this
	void OnUpdateCell( const FIntPoint& cell );

	// Calculate full path or update part of path (if grid has changed)
	void CalculateOrUpdate();

private:
	UPROPERTY()
	TObjectPtr<UDStarLite> DStarLite_;

	UPROPERTY()
	TWeakObjectPtr<AGridManager> Grid_;

	bool bInitialized = false;

	FIntPoint Start_;
	FIntPoint Goal_;
	TArray<FIntPoint> PathPoints_;
};
