// GridCell.h

#pragma once

#include "CoreMinimal.h"
#include "Building/Building.h"   
#include "GridCell.generated.h"


USTRUCT( BlueprintType )
struct FGridCell
{
	GENERATED_BODY()

  public:
	FGridCell()
		: GridCoords( FIntPoint::ZeroValue ), bIsOccupied( false ), bIsBuildable( true ), PathCost( 1.0f ),
		  BuildBonus( 0.0f ), Occupant( nullptr )
	{
	}

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Grid" )
	FIntPoint GridCoords;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Grid" )
	bool bIsOccupied;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Grid" )
	bool bIsBuildable;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Grid|Pathfinding" )
	float PathCost;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Grid|Bonus" )
	float BuildBonus;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Grid" )
	TWeakObjectPtr<ABuilding> Occupant;

	FORCEINLINE void SetCoords( const int32 x, const int32 y )
	{
		GridCoords.X = x;
		GridCoords.Y = y;
	}

	FORCEINLINE int32 GetX() const
	{
		return GridCoords.X;
	}

	FORCEINLINE int32 GetY() const
	{
		return GridCoords.Y;
	}

	FORCEINLINE void ResetRuntimeState()
	{
		bIsOccupied = false;
		Occupant	= nullptr;
	}

	bool IsFree() const
	{
		return !bIsOccupied && bIsBuildable;
	}
};
