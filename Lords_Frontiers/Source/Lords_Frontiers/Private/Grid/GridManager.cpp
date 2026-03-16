
#include "Grid/GridManager.h"

#include "DrawDebugHelpers.h"

AGridManager::AGridManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGridManager::BeginPlay()
{
	Super::BeginPlay();

	InitializeGrid();
}

int32 AGridManager::GetGridHeight() const
{
	return GridRows_.Num();
}

int32 AGridManager::GetRowWidth( const int32 y ) const
{
	if ( y < 0 || y >= GridRows_.Num() )
	{
		return 0;
	}

	return GridRows_[y].Cells.Num();
}

int32 AGridManager::GetMaxWidth() const
{
	int32 MaxWidth = 0;

	for ( const FGridRow& Row : GridRows_ )
	{
		MaxWidth = FMath::Max( MaxWidth, Row.Cells.Num() );
	}

	return MaxWidth;
}

int32 AGridManager::GetGridWidth() const
{
	return GetMaxWidth();
}

void AGridManager::SetGridVisible( const bool bVisible )
{
	if ( bGridVisible_ == bVisible )
	{
		return;
	}

	bGridVisible_ = bVisible;

	UWorld* world = GetWorld();
	if ( !world )
	{
		return;
	}

	FlushPersistentDebugLines( world );
}

bool AGridManager::IsValidCoords( const int32 x, const int32 y ) const
{
	if ( y < 0 || y >= GridRows_.Num() )
	{
		return false;
	}

	const int32 rowWidth = GridRows_[y].Cells.Num();
	return x >= 0 && x < rowWidth;
}

FGridCell* AGridManager::GetCell( const int32 x, const int32 y )
{
	if ( !IsValidCoords( x, y ) )
	{
		UE_LOG( LogTemp, Warning, TEXT( "GridManager::GetCell: coords out of bounds (%d, %d)" ), x, y );
		return nullptr;
	}

	return &GridRows_[y].Cells[x];
}

const FGridCell* AGridManager::GetCell( const int32 x, const int32 y ) const
{
	if ( !IsValidCoords( x, y ) )
	{
		UE_LOG( LogTemp, Warning, TEXT( "GridManager::GetCell (const): coords out of bounds (%d, %d)" ), x, y );
		return nullptr;
	}

	return &GridRows_[y].Cells[x];
}

bool AGridManager::GetCellWorldCenter( const FIntPoint& coords, FVector& outLocation ) const
{
	if ( !IsValidCoords( coords.X, coords.Y ) )
	{
		return false;
	}

	const float cellSize = GetCellSize();
	const FVector origin = GetActorLocation();

	const float centerX = origin.X + ( static_cast<float>( coords.X ) + 0.5f ) * cellSize;
	const float centerY = origin.Y + ( static_cast<float>( coords.Y ) + 0.5f ) * cellSize;

	outLocation = FVector( centerX, centerY, origin.Z );
	return true;
}

FIntPoint AGridManager::GetCellCoords( FVector location ) const
{
	const FIntPoint coords = GetCellCoordsRaw( location );

	const int32 height = GetGridHeight();
	const int32 width = GetRowWidth( 0 ); // assuming rectangular grid

	if ( coords.X < 0 || coords.X >= width || coords.Y < 0 || coords.Y >= height )
	{
		return { -1, -1 };
	}
	return coords;
}

FIntPoint AGridManager::GetClosestCellCoords( FVector location ) const
{
	FIntPoint coords = GetCellCoordsRaw( location );

	const int32 height = GetGridHeight();
	const int32 width = GetRowWidth( 0 ); // assuming rectangular grid

	coords.Y = FMath::Clamp( coords.Y, 0, height - 1 );
	coords.X = FMath::Clamp( coords.X, 0, width - 1 );

	return coords;
}

FIntPoint AGridManager::FindClosestWalkableCell( const FVector& location ) const
{
	FIntPoint currentCoords = GetCellCoords( location );
	if ( !IsValidCoords( currentCoords.X, currentCoords.Y ) )
	{
		return { -1, -1 };
	}

	constexpr int maxDistance = 100;

	FIntPoint closestCoords = { -1, -1 };
	float distToClosest = std::numeric_limits<float>::max();
	bool found = false;

	auto processCell = [this, &closestCoords, &location, &distToClosest, &found]( const FIntPoint coords )
	{
		const FGridCell* cell = GetCell( coords.X, coords.Y );
		if ( cell && cell->bIsWalkable )
		{
			FVector cellLocation;
			GetCellWorldCenter( coords, cellLocation );
			const float distToCell = FVector::Distance( location, cellLocation );
			if ( !found || distToCell < distToClosest )
			{
				closestCoords = coords;
				distToClosest = distToCell;
				found = true;
			}
		}
	};

	processCell( currentCoords );
	if ( found )
	{
		return currentCoords;
	}

	for ( int dist = 1; dist < maxDistance; ++dist )
	{
		for ( int dx = -dist; dx <= dist; ++dx )
		{
			processCell( { currentCoords.X + dx, currentCoords.Y + dist } );
		}

		for ( int dy = dist - 1; dy >= -dist; --dy )
		{
			processCell( { currentCoords.X + dist, currentCoords.Y + dy } );
		}

		for ( int dx = dist - 1; dx >= -dist; --dx )
		{
			processCell( { currentCoords.X + dx, currentCoords.Y - dist } );
		}

		for ( int dy = -dist + 1; dy <= dist - 1; ++dy )
		{
			processCell( { currentCoords.X - dist, currentCoords.Y + dy } );
		}

		if ( found )
		{
			return closestCoords;
		}
	}
	return { -1, -1 };
}

FIntPoint AGridManager::GetCellCoordsRaw( FVector location ) const
{
	location -= GetActorLocation();
	const int32 y = static_cast<int32>( location.Y / CellSize_ );
	const int32 x = static_cast<int32>( location.X / CellSize_ );
	return FIntPoint( x, y );
}

TArray<FGridCell*> AGridManager::GetCellsInCross( const FIntPoint& myCell, const int32 radius )
{
	TArray<FGridCell*> cellNeighbors;

	const int32 cx = myCell.X;
	const int32 cy = myCell.Y;

	for ( int32 i = 1; i <= radius; ++i )
	{
		if ( IsValidCoords( cx + i, cy ) )
		{
			cellNeighbors.Add( GetCell( cx + i, cy ) );
		}
		if ( IsValidCoords( cx - i, cy ) )
		{
			cellNeighbors.Add( GetCell( cx - i, cy ) );
		}
		if ( IsValidCoords( cx, cy + i ) )
		{
			cellNeighbors.Add( GetCell( cx, cy + i ) );
		}
		if ( IsValidCoords( cx, cy - i ) )
		{
			cellNeighbors.Add( GetCell( cx, cy - i ) );
		}
	}
	return cellNeighbors;
}

TArray<FGridCell*> AGridManager::GetCellsInSquare( const FIntPoint& myCell, const int32 radius )
{
	TArray<FGridCell*> cellNeighbors;
	const int32 cx = myCell.X;
	const int32 cy = myCell.Y;
	for ( int32 dy = -radius; dy <= radius; ++dy )
	{
		for ( int32 dx = -radius; dx <= radius; ++dx )
		{
			if ( dx == 0 && dy == 0 )
			{
				continue;
			}
			const int32 x = cx + dx;
			const int32 y = cy + dy;
			if ( IsValidCoords( x, y ) )
			{
				cellNeighbors.Add( GetCell( x, y ) );
			}
		}
	}
	return cellNeighbors;
}

TArray<FGridCell*> AGridManager::GetCellsByShape( const FIntPoint& myCell, const int32 radius, EBonusShape shape )
{
	switch ( shape )
	{
	case EBonusShape::Cross:
		return GetCellsInCross( myCell, radius );
	case EBonusShape::Square:
		return GetCellsInSquare( myCell, radius );
	default:
		return GetCellsInCross( myCell, radius );
	}
}

void AGridManager::InitializeGrid()
{
	const int32 height = GetGridHeight();

	if ( height == 0 )
	{
		UE_LOG( LogTemp, Warning, TEXT( "GridManager: GridRows is empty" ) );
		return;
	}

	for ( int32 y = 0; y < height; ++y )
	{
		const int32 rowWidth = GridRows_[y].Cells.Num();

		for ( int32 x = 0; x < rowWidth; ++x )
		{
			FGridCell& cell = GridRows_[y].Cells[x];

			cell.SetCoords( x, y );

			if ( cell.Occupant.IsValid() )
			{
				FVector center;
				if ( GetCellWorldCenter( cell.GridCoords, center ) )
				{
					cell.Occupant->SetActorLocation( center );
				}
				cell.bIsOccupied = true;
			}
			else
			{
				cell.ResetRuntimeState();
			}
		}
	}
}
