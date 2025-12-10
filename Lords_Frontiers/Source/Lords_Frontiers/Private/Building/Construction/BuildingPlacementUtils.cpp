// BuildingPlacementUtils.cpp

#include "Building/Construction/BuildingPlacementUtils.h"

#include "Building/Building.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Grid/GridManager.h"
#include "Grid/GridVisualizer.h"

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

	// 1) Позиция мыши на экране.
	float screenX = 0.0f;
	float screenY = 0.0f;
	if ( !playerController->GetMousePosition( screenX, screenY ) )
	{
		return result;
	}

	// 2) Девпроекция в луч в мире.
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

	// 3) Плоскость сетки: Z = GridOrigin.Z (сетка горизонтальная).
	const FVector gridOrigin = gridManager->GetActorLocation();
	const float planeZ = gridOrigin.Z;

	const float dirZ = worldDirection.Z;
	if ( FMath::IsNearlyZero( dirZ ) )
	{
		// Луч почти параллелен плоскости.
		return result;
	}

	const float t = ( planeZ - worldOrigin.Z ) / dirZ;
	if ( t <= 0.0f )
	{
		// Пересечение позади камеры.
		return result;
	}

	const FVector hitPoint = worldOrigin + worldDirection * t;

	// 4) Переводим точку в координаты сетки.
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

	// 5) Центр клетки берём из визуализатора, чтобы совпадать с нарисованной сеткой.
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

	// Нельзя строить на небилдабельной или уже занятой клетке.
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

	// Для отладки — рисуем небольшой бокс в центре клетки.
	DrawDebugBox( world, worldLocation, FVector( 15.0f, 15.0f, 15.0f ), FColor::Cyan, false, 2.0f );

	// Помечаем клетку занятой.
	FGridCell* cell = gridManager->GetCell( cellCoords.X, cellCoords.Y );
	if ( cell )
	{
		cell->bIsOccupied = true;
		cell->Occupant = building;
	}

	return building;
}
