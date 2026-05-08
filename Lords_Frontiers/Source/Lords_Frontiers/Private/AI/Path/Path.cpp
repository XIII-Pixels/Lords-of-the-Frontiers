// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Path/Path.h"

#include "AI/Path/SplinePointConnector.h"
#include "AI/UnitAIManager.h"
#include "Core/CoreManager.h"
#include "Grid/GridManager.h"

#include "Kismet/GameplayStatics.h"

UPath::UPath()
{
}

void UPath::PostInitProperties()
{
	Super::PostInitProperties();
	DStarLite_ = NewObject<UDStarLite>( this );
}

void UPath::Initialize( const FPathConfig& config )
{
	DStarLite_->Initialize( config );
}

void UPath::OnUpdateCell( const FIntPoint& cell )
{
	DStarLite_->OnUpdateEdgeCost( cell );
}

void UPath::CalculateOrUpdate()
{
	DStarLite_->ComputeShortestPath();
	PathPoints_ = DStarLite_->GetPath();
	RebuildSpline();
}

void UPath::RebuildSpline()
{
	if ( Spline_ )
	{
		ClearSpline();
	}

	float groundHeight = 0.f;
	const AGridManager* grid = nullptr;
	TSubclassOf<ASplinePointConnector> splineClass;

	if ( const UCoreManager* cm = UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<UCoreManager>() )
	{
		if ( const AUnitAIManager* unitAIManager = cm->GetUnitAIManager() )
		{
			groundHeight = unitAIManager->GroundHeight() + 5.f;
			splineClass = unitAIManager->SplineClass();
		}
		grid = cm->GetGridManager();
	}

	if ( !splineClass )
	{
		UE_LOG( LogTemp, Error, TEXT( "UPath: no spline class found. Cannot build spline" ) );
		Spline_ = nullptr;
	}

	Spline_ = GetWorld()->SpawnActor<ASplinePointConnector>( splineClass );
	if ( !IsValid( Spline_ ) )
	{
		UE_LOG( LogTemp, Error, TEXT( "UPath: failed to create spline" ) );
		return;
	}

	for ( const FIntPoint& point : PathPoints_ )
	{
		FVector worldLocation;
		if ( grid )
		{
			grid->GetCellWorldCenter( point, worldLocation );
		}

		Spline_->AddPoint( FVector( worldLocation.X, worldLocation.Y, groundHeight ) );
	}

	Spline_->BuildSpline();
}

const TArray<FIntPoint>& UPath::GetPoints() const
{
	return PathPoints_;
}

void UPath::RemovePoint( int index )
{
	PathPoints_.RemoveAt( index );
}

void UPath::ClearSpline()
{
	if ( Spline_ )
	{
		Spline_->Clear();
	}
}
