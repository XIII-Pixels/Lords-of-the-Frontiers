#pragma once

#include "GridCell.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "GridManager.generated.h"

/// @brief Single row of the 2D grid.
/// GridRows[Y].Cells[X] is the cell with coordinates (X, Y), where Y is the row
/// index.
USTRUCT( BlueprintType )
struct FGridRow
{
	GENERATED_BODY()

	// Cells of this row along the X axis.
	UPROPERTY( EditAnywhere, Category = "Settings|Grid" )
	TArray<FGridCell> Cells;
};

/// @brief Manager of the logical building grid.
/// Stores a 2D matrix of cells and basic grid parameters.
/// Supports non-rectangular grids where each row can have a different width.
UCLASS()
class LORDS_FRONTIERS_API AGridManager : public AActor
{
	GENERATED_BODY()

public:
	AGridManager();

	// === Public grid parameter getters ===

	/// @brief Get the maximum row width across all rows.
	/// Useful for visualization bounds.
	/// @return Maximum number of cells in any row, or 0 if grid is empty.
	int32 GetMaxWidth() const;

	/// @brief Get grid width in cells (number of columns).
	/// For non-rectangular grids returns the maximum row width.
	/// @return Grid width in cells.
	int32 GetGridWidth() const;

	/// @brief Get the width (number of cells) of a specific row.
	/// @param[in] y Row index.
	/// @return Number of cells in row y, or 0 if y is out of bounds.
	int32 GetRowWidth( const int32 y ) const;

	/// @brief Get grid height in cells (number of rows).
	/// @return Grid height in cells.
	int32 GetGridHeight() const;

	/// @brief Get cell size in centimeters.
	/// @return Cell size in centimeters.
	float GetCellSize() const
	{
		return CellSize_;
	}

	/// @brief Check whether the given cell coordinates are within the current
	/// @param[in] x X coordinate (column index).
	/// @param[in] y Y coordinate (row index).
	/// @return true if coordinates are valid, false otherwise.
	bool IsValidCoords( const int32 x, const int32 y ) const;

	/// @brief Check whether grid visualization is enabled.
	/// @return true if the grid is visible, false otherwise.
	bool IsGridVisible() const
	{
		return bGridVisible_;
	}

	/// @brief Enable or disable visual grid rendering.
	/// @param[in] bVisible Whether the grid should be visible.
	void SetGridVisible( const bool bVisible );

	// === Access to grid cells ===

	/// @brief Get cell by coordinates (X, Y).
	/// @param[in] x X coordinate (column index).
	/// @param[in] y Y coordinate (row index).
	/// @return Pointer to the cell or nullptr if coordinates are out of bounds.
	FGridCell* GetCell( const int32 x, const int32 y );

	/// @brief Get const cell by coordinates (X, Y).
	/// @param[in] x X coordinate (column index).
	/// @param[in] y Y coordinate (row index).
	/// @return Const pointer to the cell or nullptr if coordinates are out of
	/// bounds.
	const FGridCell* GetCell( const int32 x, const int32 y ) const;

	/// @brief Calculate point world location based on grid coords with z = grid z.
	/// Works correctly for non-rectangular grids.
	bool GetCellWorldCenter( const FIntPoint& cellCoords, FVector& outLocation ) const;

	/// @brief Calculate grid coords based on location.
	/// For non-rectangular grids, clamps X to the actual row width.
	FIntPoint GetClosestCellCoords( FVector location ) const;

	/// @brief Get array the cell's neighbors
	/// @param[in] FGridCell 
	/// @param[in] radius how many cells to look at.
	/// @return Array of cells TArray<FGridCell*>
	/// 

	TArray<FGridCell*> GetCellsInCross( const FIntPoint& myCell, int32 radius );
	TArray<FGridCell*> GetCellsInSquare( const FIntPoint& myCell, int32 radius );
	TArray<FGridCell*> GetCellsByShape( const FIntPoint& myCell, int32 radius, EBonusShape shape );

protected:
	/// @brief Called when the game starts or when spawned.
	virtual void BeginPlay() override;

	// === Grid settings ===

	/// @brief Cell size in centimeters (world scale of the grid).
	UPROPERTY( EditAnywhere, Category = "Settings|Grid", meta = ( AllowPrivateAccess = "true" ) )
	float CellSize_ = 100.0f;

	/// @brief Flag that controls visual grid rendering.
	UPROPERTY( EditAnywhere, Category = "Settings|Grid", meta = ( AllowPrivateAccess = "true" ) )
	bool bGridVisible_ = true;

	/// @brief 2D matrix of cells:
	/// GridRows[Y].Cells[X] is the cell with coordinates (X, Y).
	/// Rows may have different lengths (non-rectangular grid).
	UPROPERTY( EditAnywhere, Category = "Settings|Grid", meta = ( AllowPrivateAccess = "true" ) )
	TArray<FGridRow> GridRows_;

private:
	/// @brief Initialize the grid:
	/// - sets GridCoords for each cell;
	/// - snaps pre-assigned occupants to cell centers;
	/// - resets runtime state of empty cells.
	void InitializeGrid();
};