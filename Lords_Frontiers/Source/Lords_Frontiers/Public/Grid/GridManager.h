// GridManager.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridCell.h"
#include "GridManager.generated.h"

USTRUCT( BlueprintType )
struct FGridRow
{
	GENERATED_BODY()

  public:
	UPROPERTY( EditAnywhere, Category = "Grid" )
	TArray<FGridCell> Cells;
};

UCLASS()
class LORDS_FRONTIERS_API AGridManager : public AActor
{
	GENERATED_BODY()

  public:
	AGridManager();

	int32 GetGridWidth() const;

	int32 GetGridHeight() const;

	float GetCellSize() const
	{
		return CellSize_;
	}

	bool IsValidCoords( const int32 x, const int32 y ) const;

	bool IsGridVisible() const
	{
		return bGridVisible_;
	}

	void SetGridVisible( const bool bVisible );

	FGridCell* GetCell( const int32 x, const int32 y );

	const FGridCell* GetCell( const int32 x, const int32 y ) const;

  protected:
	void BeginPlay() override;

	UPROPERTY( EditAnywhere, Category = "Grid", meta = ( AllowPrivateAccess = "true" ) )
	float CellSize_ = 100.0f;

	UPROPERTY( EditAnywhere, Category = "Grid", meta = ( AllowPrivateAccess = "true" ) )
	bool bGridVisible_ = true;

	UPROPERTY( EditAnywhere, Category = "Grid", meta = ( AllowPrivateAccess = "true" ) )
	TArray<FGridRow> GridRows_;


  private:

	void InitializeGrid();
};
