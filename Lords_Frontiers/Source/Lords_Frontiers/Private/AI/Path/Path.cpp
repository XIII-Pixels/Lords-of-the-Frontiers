// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Path/Path.h"

#include "Grid/GridManager.h"

UPath::UPath()
{
}

void UPath::PostInitProperties()
{
	Super::PostInitProperties();
	DStarLite_ = NewObject<UDStarLite>( this );
}

void UPath::SetGrid( TWeakObjectPtr<AGridManager> grid )
{
	Grid_ = grid;
	DStarLite_->SetGrid( Grid_ );
}

void UPath::SetEmptyCellTravelTime( float emptyCellTravelTime )
{
	DStarLite_->SetEmptyCellTravelTime( emptyCellTravelTime );
}

void UPath::SetUnitAttackInfo( float damage, float cooldown )
{
	DStarLite_->SetUnitAttackInfo( damage, cooldown );
}

void UPath::SetStartAndGoal( const FIntPoint& start, const FIntPoint& goal )
{
	DStarLite_->Initialize( start, goal );
	bStartGoalInitialized = true;
}

void UPath::OnUpdateCell( const FIntPoint& cell )
{
	DStarLite_->OnUpdateEdgeCost( cell );
}

void UPath::CalculateOrUpdate()
{
	if ( !Grid_.IsValid() )
	{
		UE_LOG( LogTemp, Error, TEXT( "Trying to calculate path with invalid pointer to grid" ) );
		return;
	}
	if ( !bStartGoalInitialized )
	{
		UE_LOG( LogTemp, Error, TEXT( "Trying to calculate path on uninitialized path" ) );
		return;
	}

	DStarLite_->ComputeShortestPath();
	PathPoints_ = DStarLite_->GetPath();
}

const TArray<FIntPoint>& UPath::GetPoints() const
{
	return PathPoints_;
}
