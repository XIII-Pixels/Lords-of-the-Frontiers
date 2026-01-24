// GridManager.h

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
/// Can draw a debug grid with lines for visual testing.
UCLASS()
class LORDS_FRONTIERS_API AGridManager : public AActor
{
	GENERATED_BODY()

public:
	AGridManager();

	// === Public grid parameter getters ===

	/// @brief Get grid width in cells (number of columns).
	/// @return Grid width in cells.
	int32 GetGridWidth() const;

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
	/// grid bounds.
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

	/// @brief Calculate point world location based on grid coords with z = grid z
	bool GetCellWorldCenter( const FIntPoint& cellCoords, FVector& outLocation ) const;

protected:
	/// @brief Called when the game starts or when spawned.
	virtual void BeginPlay() override;

	// === Grid settings ===

	/// @brief Cell size in centimeters (world scale of the grid).
	UPROPERTY( EditAnywhere, Category = "Settings|Grid", meta = ( AllowPrivateAccess = "true" ) )
	float CellSize_ = 100.0f;

	/// @brief Flag that controls visual grid rendering.
	/// Debug drawing is currently handled by the manager itself.
	UPROPERTY( EditAnywhere, Category = "Settings|Grid", meta = ( AllowPrivateAccess = "true" ) )
	bool bGridVisible_ = true;

	/// @brief 2D matrix of cells:
	/// GridRows[Y].Cells[X] is the cell with coordinates (X, Y).
	/// Static cell properties (bIsBuildable, PathCost, BuildBonus)
	/// are edited directly on the cells.
	UPROPERTY( EditAnywhere, Category = "Settings|Grid", meta = ( AllowPrivateAccess = "true" ) )
	TArray<FGridRow> GridRows_;

private:
	/// @brief Initialize the grid:
	/// - sets GridCoords for each cell;
	/// - resets runtime state of cells.
	void InitializeGrid();
};
