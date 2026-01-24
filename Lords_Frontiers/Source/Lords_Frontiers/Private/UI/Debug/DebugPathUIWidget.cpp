// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Debug/DebugPathUIWidget.h"

#include "AI/Path/PathPointsManager.h"
#include "Grid/GridManager.h"

#include "EngineUtils.h"

void UDebugPathUIWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( Button1 )
	{
		Button1->OnClicked.RemoveAll( this );
		Button1->OnClicked.AddDynamic( this, &UDebugPathUIWidget::OnButton1Clicked );
	}

	if ( Button2 )
	{
		Button2->OnClicked.RemoveAll( this );
		Button2->OnClicked.AddDynamic( this, &UDebugPathUIWidget::OnButton2Clicked );
	}

	TWeakObjectPtr<AGridManager> grid;
	for ( TActorIterator<AGridManager> it( GetWorld() ); it; ++it )
	{
		if ( grid.IsValid() )
		{
			// Second grid found => grid is not unique
			grid = nullptr;
			UE_LOG(
			    LogTemp, Error, TEXT( "More than one grid was found in the world. Cannot set grid for pathfinding" )
			);
			return;
		}
		grid = *it;
	}
	if ( !grid.IsValid() )
	{
		UE_LOG( LogTemp, Error, TEXT( "Grid not found. Cannot set grid for pathfinding" ) );
		return;
	}

	Path_ = NewObject<UPath>( this );
	Path_->SetGrid( grid );
	Path_->SetStartAndGoal( { 0, 0 }, { 5, 5 } );
	Path_->SetUnitAttackInfo( 10.0f, 2.0f );
	Path_->SetEmptyCellTravelTime( 1.0f );

	PathPointsManager_ = NewObject<UPathPointsManager>( this );
	PathPointsManager_->PathTargetPointClass = PathTargetPointClass_;
	PathPointsManager_->SetGrid( grid );
}

void UDebugPathUIWidget::OnButton1Clicked()
{
	Path_->CalculateOrUpdate();
	PathPointsManager_->AddPathPoints( *Path_ );
}

void UDebugPathUIWidget::OnButton2Clicked()
{
	UE_LOG( LogTemp, Log, TEXT( "Button 2 clicked" ) );
}
