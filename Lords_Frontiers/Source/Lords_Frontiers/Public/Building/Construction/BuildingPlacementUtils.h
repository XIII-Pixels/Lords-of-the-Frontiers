// BuildingPlacementUtils.h

#pragma once

#include "Grid/GridCell.h"

#include "CoreMinimal.h"

class AGridManager;
class AGridVisualizer;
class APlayerController;
class ABuilding;

struct FBuildingPlacementResult
{
	bool bHitValidGrid = false;

	FIntPoint CellCoords = FIntPoint( INDEX_NONE, INDEX_NONE );

	FVector CellWorldLocation = FVector::ZeroVector;
};

namespace BuildingPlacementUtils
{
	FBuildingPlacementResult FindCellUnderCursor(
	    APlayerController* playerController, const AGridManager* gridManager, const AGridVisualizer* gridVisualizer
	);
	bool CanBuildAtCell( const AGridManager* gridManager, const FIntPoint& cellCoords );

	ABuilding* PlaceBuilding(
	    UWorld* world, TSubclassOf<ABuilding> buildingClass, const FVector& worldLocation, AGridManager* gridManager,
	    const FIntPoint& cellCoords
	);
} // namespace BuildingPlacementUtils
