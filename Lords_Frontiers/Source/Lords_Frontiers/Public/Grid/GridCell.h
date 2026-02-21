// GridCell.h

#pragma once

#include "Building/Building.h"

#include "CoreMinimal.h"

#include "GridCell.generated.h"

/**(Maxim)
 *
 */
USTRUCT( BlueprintType )
struct FGridCell
{
	GENERATED_BODY()

	FGridCell()
	    : GridCoords( FIntPoint::ZeroValue ), bIsOccupied( false ), bIsBuildable( true ), bIsWalkable( true ),
	      BuildBonus( 0.0f ), Occupant( nullptr )
	{
	}

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Grid" )
	FIntPoint GridCoords;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Grid" )
	bool bIsOccupied;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Grid" )
	bool bIsBuildable;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Grid" )
	bool bIsWalkable;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Grid|Bonus" )
	float BuildBonus;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Grid" )
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
		Occupant = nullptr;
	}

	bool IsFree() const
	{
		return !bIsOccupied && bIsBuildable;
	}
};
