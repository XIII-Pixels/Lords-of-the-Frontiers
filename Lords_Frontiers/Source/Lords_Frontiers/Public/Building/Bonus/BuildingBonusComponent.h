// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Building/Bonus/BuildingBonusEntry.h"

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "BuildingBonusComponent.generated.h"

class AGridManager;
class ABuilding;
struct FGridCell;

/** (Maxim)
 */
struct FBonusApplication
{
	TWeakObjectPtr<ABuilding> TargetBuilding_;
	int32 EntryIndex_ = -1;
	float AppliedValue_ = 0.0f;
};

static FLinearColor GetDefaultColorForBonusType( EBonusCategory category )
{
	switch ( category )
	{
	case EBonusCategory::Production:
		return FLinearColor( 0.2f, 0.8f, 0.2f, 0.4f );
	case EBonusCategory::Maintenance:
		return FLinearColor( 0.9f, 0.8f, 0.1f, 0.4f );
	case EBonusCategory::Stats:
		return FLinearColor( 0.5f, 0.2f, 0.9f, 0.4f );
	default:
		return FLinearColor( 0.5f, 0.5f, 0.5f, 0.4f );
	}
}

UCLASS( ClassGroup = ( BonusBuild ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API UBuildingBonusComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBuildingBonusComponent();

	void RecalculateBonuses( AGridManager* gridManager, const FIntPoint& myCellCoordinate );

	void RemoveAppliedBonuses();

	void SetBonusIconVisible( bool bVisible );

	FBonusIconData GetInfoSingleBonus( int32 entryIndex, const FVector& displayLocation );

	TArray<FBuildingBonusEntry> GetBonusEntries()
	{
		return BonusEntries_;
	};

	static constexpr int32 MaxPossibleBonusRadius = 5;

	static UBuildingBonusComponent* FindInBlueprintClass( TSubclassOf<ABuilding> buildingClass );

	static TArray<FIntPoint> FindBonusCells( TSubclassOf<ABuilding> buildingClass, AGridManager* gridManager );

protected:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Bonus" )
	TArray<FBuildingBonusEntry> BonusEntries_;

private:
	static void FindCellsWhereBuildingGetsBonuses(
	    UBuildingBonusComponent* bonusComp, AGridManager* gridManager, TArray<FIntPoint>& outCells
	);

	static void
	FindCellsWhereNeighborsGetBonuses( const ABuilding* cdo, AGridManager* gridManager, TArray<FIntPoint>& outCells );

	TArray<FBonusApplication> ActiveApplications_;

	void ApplySingleBonus( ABuilding* target, int32 entryIndex );

	void RevertSingleBonus( const FBonusApplication& application );

	static bool HasMatchingNeighbor(
	    AGridManager* gridManager, const FIntPoint& candidate, const TArray<FBuildingBonusEntry>& entries
	);

	static void CollectRadiusCells(
	    UBuildingBonusComponent* bonusComp, const ABuilding* cdo, AGridManager* gridManager,
	    const FIntPoint& buildingCell, TArray<FIntPoint>& outCells
	);
};
