#include "Building/Bonus/BuildingBonusComponent.h"

#include "Building/Bonus/BuildingBonusEntry.h"
#include "Building/Building.h"
#include "Building/ResourceBuilding.h"
#include "EntityStats.h"
#include "Grid/GridCell.h"
#include "Grid/GridManager.h"

#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
UBuildingBonusComponent::UBuildingBonusComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
// GetBonusCellsWithColors
void UBuildingBonusComponent::RecalculateBonuses( AGridManager* gridManager, const FIntPoint& myCellCoordinate )
{
	RemoveAppliedBonuses();

	FGridCell* myCell = gridManager->GetCell( myCellCoordinate.X, myCellCoordinate.Y );
	if ( !myCell )
	{
		return;
	}

	ABuilding* target = Cast<ABuilding>( GetOwner() );
	if ( !target )
	{
		return;
	}

	for ( int32 i = 0; i < BonusEntries_.Num(); ++i )
	{
		TArray<FGridCell*> neighborsCells = gridManager->GetCellsInRadius( myCell, BonusEntries_[i].Radius );
		for ( const FGridCell* cell : neighborsCells )
		{
			ABuilding* occupant = Cast<ABuilding>( cell->Occupant.Get() );
			if ( occupant && occupant->IsA( BonusEntries_[i].SourceBuildingClass ) )
			{
				ApplySingleBonus( target, i );
			}
		}
	}
}

void UBuildingBonusComponent::RemoveAppliedBonuses()
{
	for ( const auto& application : ActiveApplications_ )
	{
		RevertSingleBonus( application );
	}
	ActiveApplications_.Empty();
}

void UBuildingBonusComponent::ApplySingleBonus( ABuilding* target, int32 entryIndex )
{

	if ( !IsValid( target ) )
	{
		return;
	}

	if ( !BonusEntries_.IsValidIndex( entryIndex ) )
	{
		return;
	}

	const FBuildingBonusEntry& entry = BonusEntries_[entryIndex];
	bool activeBonus = false;
	switch ( entry.Category )
	{
	case EBonusCategory::Production:
		if ( AResourceBuilding* resBuilding = Cast<AResourceBuilding>( target ) )
		{
			resBuilding->ModifyProduction( entry.ResourceType, entry.Value );
			activeBonus = true;
		}
		break;

	case EBonusCategory::Maintenance:
		target->ModifyMaintenanceCost( entry.ResourceType, entry.Value );
		activeBonus = true;
		break;

	case EBonusCategory::Stats:
		target->Stats().AddStat( entry.StatType, entry.Value );
		activeBonus = true;
		break;
	}
	if ( activeBonus )
	{
		ActiveApplications_.Add( { target, entryIndex, entry.Value } );
	}
}

FBonusIconData UBuildingBonusComponent::GetInfoSingleBonus( int32 entryIndex, const FVector& displayLocation )
{
	FBonusIconData data;

	if ( !BonusEntries_.IsValidIndex( entryIndex ) )
	{
		return data;
	}

	const FBuildingBonusEntry& entry = BonusEntries_[entryIndex];
	data.WorldLocation = displayLocation;
	data.Icon = entry.BonusIcon;
	data.Value = entry.Value;
	data.Category = entry.Category;
	data.ResourceType = entry.ResourceType;
	data.StatType = entry.StatType;
	return data;
}

void UBuildingBonusComponent::RevertSingleBonus( const FBonusApplication& application )
{
	ABuilding* targetRaw = application.TargetBuilding_.Get();
	if ( !targetRaw )
	{
		return;
	}

	if ( !BonusEntries_.IsValidIndex( application.EntryIndex_ ) )
	{
		return;
	}

	const FBuildingBonusEntry& entry = BonusEntries_[application.EntryIndex_];

	switch ( entry.Category )
	{
	case EBonusCategory::Production:
		if ( AResourceBuilding* resBuilding = Cast<AResourceBuilding>( targetRaw ) )
		{
			resBuilding->ModifyProduction( entry.ResourceType, ( application.AppliedValue_ * -1 ) );
		}
		break;

	case EBonusCategory::Maintenance:
		targetRaw->ModifyMaintenanceCost( entry.ResourceType, ( application.AppliedValue_ * -1 ) );
		break;

	case EBonusCategory::Stats:
		targetRaw->Stats().AddStat( entry.StatType, ( application.AppliedValue_ * -1 ) );
		break;
	}
}

void UBuildingBonusComponent::SetBonusIconVisible( bool bVisible )
{
	return;
}

UBuildingBonusComponent* UBuildingBonusComponent::FindInBlueprintClass( TSubclassOf<ABuilding> buildingClass )
{
	UBlueprintGeneratedClass* bpClass = Cast<UBlueprintGeneratedClass>( buildingClass.Get() );
	if ( !bpClass || !bpClass->SimpleConstructionScript )
	{
		return nullptr;
	}

	for ( USCS_Node* node : bpClass->SimpleConstructionScript->GetAllNodes() )
	{
		if ( !node )
		{
			continue;
		}
		UBuildingBonusComponent* comp = Cast<UBuildingBonusComponent>( node->ComponentTemplate );
		if ( comp )
		{
			return comp;
		}
	}
	return nullptr;
}

TArray<FIntPoint>
UBuildingBonusComponent::FindBonusCells( TSubclassOf<ABuilding> buildingClass, AGridManager* gridManager )
{
	TArray<FIntPoint> result;

	if ( !buildingClass || !gridManager )
	{
		return result;
	}

	UBuildingBonusComponent* bonusComp = FindInBlueprintClass( buildingClass );
	if ( bonusComp )
	{
		FindCellsWhereBuildingGetsBonuses( bonusComp, gridManager, result );
	}

	const ABuilding* cdo = buildingClass->GetDefaultObject<ABuilding>();
	FindCellsWhereNeighborsGetBonuses( cdo, gridManager, result );

	return result;
}

void UBuildingBonusComponent::FindCellsWhereBuildingGetsBonuses(
    UBuildingBonusComponent* bonusComp, AGridManager* gridManager, TArray<FIntPoint>& outCells
)
{
	const TArray<FBuildingBonusEntry>& entries = bonusComp->GetBonusEntries();
	const int32 gridHeight = gridManager->GetGridHeight();

	for ( int32 y = 0; y < gridHeight; ++y )
	{
		const int32 rowWidth = gridManager->GetRowWidth( y );
		for ( int32 x = 0; x < rowWidth; ++x )
		{
			FIntPoint candidate( x, y );

			if ( HasMatchingNeighbor( gridManager, candidate, entries ) )
			{
				outCells.AddUnique( candidate );
			}
		}
	}
}

void UBuildingBonusComponent::FindCellsWhereNeighborsGetBonuses(
    const ABuilding* cdo, AGridManager* gridManager, TArray<FIntPoint>& outCells
)
{
	const int32 gridHeight = gridManager->GetGridHeight();

	for ( int32 y = 0; y < gridHeight; ++y )
	{
		const int32 rowWidth = gridManager->GetRowWidth( y );
		for ( int32 x = 0; x < rowWidth; ++x )
		{
			FGridCell* cell = gridManager->GetCell( x, y );
			if ( !cell || !cell->bIsOccupied )
			{
				continue;
			}

			ABuilding* building = Cast<ABuilding>( cell->Occupant.Get() );
			if ( !building )
			{
				continue;
			}

			UBuildingBonusComponent* bonusComp = building->FindComponentByClass<UBuildingBonusComponent>();
			if ( !bonusComp )
			{
				continue;
			}

			CollectRadiusCells( bonusComp, cdo, gridManager, FIntPoint( x, y ), outCells );
		}
	}
}

bool UBuildingBonusComponent::HasMatchingNeighbor(
    AGridManager* gridManager, const FIntPoint& candidate, const TArray<FBuildingBonusEntry>& entries
)
{
	FGridCell* candidateCell = gridManager->GetCell( candidate.X, candidate.Y );
	if ( !candidateCell || !candidateCell->bIsBuildable || candidateCell->bIsOccupied )
	{
		return false;
	}

	for ( const FBuildingBonusEntry& entry : entries )
	{
		TArray<FGridCell*> neighbors = gridManager->GetCellsInRadius( candidate, entry.Radius );

		for ( const FGridCell* cell : neighbors )
		{
			if ( !cell || !cell->bIsOccupied )
			{
				continue;
			}

			ABuilding* occupant = Cast<ABuilding>( cell->Occupant.Get() );
			if ( occupant && occupant->IsA( entry.SourceBuildingClass ) )
			{
				return true;
			}
		}
	}
	return false;
}

void UBuildingBonusComponent::CollectRadiusCells(
    UBuildingBonusComponent* bonusComp, const ABuilding* cdo, AGridManager* gridManager, const FIntPoint& buildingCell,
    TArray<FIntPoint>& outCells
)
{

	const TArray<FBuildingBonusEntry>& entries = bonusComp->GetBonusEntries();

	for ( const FBuildingBonusEntry& entry : entries )
	{
		if ( !cdo->IsA( entry.SourceBuildingClass ) )
		{
			continue;
		}

		TArray<FGridCell*> cells = gridManager->GetCellsInRadius( buildingCell, entry.Radius );

		for ( const FGridCell* cell : cells )
		{
			if ( cell && cell->bIsBuildable )
			{
				outCells.AddUnique( cell->GridCoords );
			}
		}
	}
}