// BuildingPlacementUtils.cpp

#include "Building/Construction/BuildingPlacementUtils.h"

#include "Building/Building.h"
#include "DrawDebugHelpers.h"
#include "Grid/GridManager.h"
#include "Grid/GridVisualizer.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

FBuildingPlacementResult BuildingPlacementUtils::FindCellUnderCursor(
    APlayerController* playerController, const AGridManager* gridManager, const AGridVisualizer* gridVisualizer
)
{
	FBuildingPlacementResult result;

	if ( !playerController || !gridManager || !gridVisualizer )
	{
		return result;
	}

	UWorld* world = playerController->GetWorld();
	if ( !world )
	{
		return result;
	}

	float screenX = 0.0f;
	float screenY = 0.0f;
	if ( !playerController->GetMousePosition( screenX, screenY ) )
	{
		return result;
	}

	FVector worldOrigin;
	FVector worldDirection;
	if ( !playerController->DeprojectScreenPositionToWorld( screenX, screenY, worldOrigin, worldDirection ) )
	{
		return result;
	}

	worldDirection = worldDirection.GetSafeNormal();
	if ( worldDirection.IsNearlyZero() )
	{
		return result;
	}

	const FVector gridOrigin = gridManager->GetActorLocation();
	const float planeZ = gridOrigin.Z;

	const float dirZ = worldDirection.Z;
	if ( FMath::IsNearlyZero( dirZ ) )
	{
		return result;
	}

	const float t = ( planeZ - worldOrigin.Z ) / dirZ;
	if ( t <= 0.0f )
	{
		return result;
	}

	const FVector hitPoint = worldOrigin + worldDirection * t;

	const float cellSize = gridManager->GetCellSize();
	if ( cellSize <= KINDA_SMALL_NUMBER )
	{
		return result;
	}

	const float localX = hitPoint.X - gridOrigin.X;
	const float localY = hitPoint.Y - gridOrigin.Y;

	const int32 gridX = FMath::FloorToInt( localX / cellSize );
	const int32 gridY = FMath::FloorToInt( localY / cellSize );

	const FGridCell* cell = gridManager->GetCell( gridX, gridY );
	if ( !cell )
	{
		return result;
	}

	const FIntPoint cellCoords( gridX, gridY );

	FVector cellWorldLocation;
	if ( !gridVisualizer->GetCellWorldCenter( cellCoords, cellWorldLocation ) )
	{
		return result;
	}

	result.bHitValidGrid = true;
	result.CellCoords = cellCoords;
	result.CellWorldLocation = cellWorldLocation;

	return result;
}

bool BuildingPlacementUtils::CanBuildAtCell( const AGridManager* gridManager, const FIntPoint& cellCoords )
{
	if ( !gridManager )
	{
		return false;
	}

	const FGridCell* cell = gridManager->GetCell( cellCoords.X, cellCoords.Y );
	if ( !cell )
	{
		return false;
	}

	if ( !cell->bIsBuildable || cell->bIsOccupied )
	{
		return false;
	}

	return true;
}

ABuilding* BuildingPlacementUtils::PlaceBuilding(
    UWorld* world, TSubclassOf<ABuilding> buildingClass, const FVector& worldLocation, AGridManager* gridManager,
    const FIntPoint& cellCoords
)
{
	if ( !world || !gridManager || !*buildingClass )
	{
		return nullptr;
	}

	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ABuilding* building =
	    world->SpawnActor<ABuilding>( buildingClass, worldLocation, FRotator::ZeroRotator, spawnParams );

	if ( !building )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
			    -1, 2.0f, FColor::Red, TEXT( "BuildingPlacementUtils: failed to spawn building" )
			);
		}
		return nullptr;
	}

	DrawDebugBox( world, worldLocation, FVector( 15.0f, 15.0f, 15.0f ), FColor::Cyan, false, 2.0f );

	FGridCell* cell = gridManager->GetCell( cellCoords.X, cellCoords.Y );
	if ( cell )
	{
		cell->bIsOccupied = true;
		cell->Occupant = building;
	}

	return building;
}
