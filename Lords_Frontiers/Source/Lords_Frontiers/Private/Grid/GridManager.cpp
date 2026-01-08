// GridManager.cpp

#include "Grid/GridManager.h"

#include "DrawDebugHelpers.h"

// Grid manager constructor.
// Tick is disabled because the grid does not need per-frame updates.
AGridManager::AGridManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts.
// Prepares the grid for use.
void AGridManager::BeginPlay()
{
	Super::BeginPlay();

	InitializeGrid();
}

// Returns grid height (number of rows).
int32 AGridManager::GetGridHeight() const
{
	return GridRows_.Num();
}

// Returns grid width (number of columns).
// If there are no rows, width is considered zero.
int32 AGridManager::GetGridWidth() const
{
	if ( GridRows_.Num() == 0 )
	{
		return 0;
	}

	return GridRows_[0].Cells.Num();
}

// Enable or disable visual grid debug rendering.
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

	// Currently we only clear persistent debug lines when toggling the flag.
	// This is a temporary solution for tests.
	FlushPersistentDebugLines( world );
}

// Checks whether the given coordinates are within the grid bounds.
bool AGridManager::IsValidCoords( const int32 x, const int32 y ) const
{
	const int32 width = GetGridWidth();
	const int32 height = GetGridHeight();

	if ( width <= 0 || height <= 0 )
	{
		return false;
	}

	return x >= 0 && x < width && y >= 0 && y < height;
}

// Returns a pointer to the cell at coordinates (x, y).
// Returns nullptr if the coordinates are out of bounds.
FGridCell* AGridManager::GetCell( const int32 x, const int32 y )
{
	if ( !IsValidCoords( x, y ) )
	{
		UE_LOG( LogTemp, Warning, TEXT( "GridManager::GetCell: coords out of bounds (%d, %d)" ), x, y );
		return nullptr;
	}

	return &GridRows_[y].Cells[x];
}

// Const version of GetCell.
// Returns nullptr if the coordinates are out of bounds.
const FGridCell* AGridManager::GetCell( const int32 x, const int32 y ) const
{
	if ( !IsValidCoords( x, y ) )
	{
		UE_LOG( LogTemp, Warning, TEXT( "GridManager::GetCell (const): coords out of bounds (%d, %d)" ), x, y );
		return nullptr;
	}

	return &GridRows_[y].Cells[x];
}

// Initializes the grid:
//  - sets GridCoords;
//  - resets runtime state (occupancy and actor);
//  - validates basic matrix consistency.
void AGridManager::InitializeGrid()
{
	const int32 height = GetGridHeight();

	if ( height == 0 )
	{
		UE_LOG( LogTemp, Warning, TEXT( "GridManager: GridRows is empty" ) );
		return;
	}

	const int32 width = GetGridWidth();

	if ( width == 0 )
	{
		UE_LOG( LogTemp, Warning, TEXT( "GridManager: first row has no cells" ) );
		return;
	}

	for ( int32 y = 0; y < height; ++y )
	{
		const int32 rowWidth = GridRows_[y].Cells.Num();

		if ( rowWidth != width )
		{
			UE_LOG(
			    LogTemp, Warning,
			    TEXT( "GridManager: row %d has different width (%d) than first row "
			          "(%d)" ),
			    y, rowWidth, width
			);
		}

		const int32 maxX = FMath::Min( width, rowWidth );

		for ( int32 x = 0; x < maxX; ++x )
		{
			FGridCell& cell = GridRows_[y].Cells[x];

			// Set cell coordinates.
			cell.SetCoords( x, y );

			// Reset only runtime state (occupancy and occupant).
			cell.ResetRuntimeState();
		}
	}
}
