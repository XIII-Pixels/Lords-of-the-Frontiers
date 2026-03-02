// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/UnitAIManager.h"

#include "AI/Path/PathPointsManager.h"
#include "AI/TargetBuildingTracker.h"
#include "Grid/GridManager.h"

#include "Kismet/GameplayStatics.h"

AUnitAIManager::AUnitAIManager()
{
}

void AUnitAIManager::OnPreWaveStart()
{
	if ( IsValid( TargetBuildingTracker_ ) )
	{
		TargetBuildingTracker_->ScanLevelForBuildings();
	}
}

const UPathPointsManager* AUnitAIManager::PathPointsManager()
{
	return PathPointsManager_;
}

const UTargetBuildingTracker* AUnitAIManager::TargetBuildingTracker()
{
	return TargetBuildingTracker_;
}

void AUnitAIManager::BeginPlay()
{
	Super::BeginPlay();

	const UWorld* world = GetWorld();
	if ( !world )
	{
		UE_LOG( LogTemp, Error, TEXT( "AUnitAIManager: world is null" ) );
		return;
	}

	PathPointsManager_ = NewObject<UPathPointsManager>( this );
	TargetBuildingTracker_ = NewObject<UTargetBuildingTracker>( this );

	AGridManager* gridManager =
	    Cast<AGridManager>( UGameplayStatics::GetActorOfClass( world, AGridManager::StaticClass() ) );

	// PathPointsManager settings

	if ( gridManager )
	{
		PathPointsManager_->SetGrid( gridManager );
	}
	else
	{
		UE_LOG( LogTemp, Error, TEXT( "AUnitAIManager: grid manager is not found" ) );
	}

	PathPointsManager_->SetGoalActor( GoalActor );
	PathPointsManager_->SetPathTargetPointClass( PathPointClass );
	PathPointsManager_->SetPointReachRadius( PathPointReachRadius );

	// TargetBuildingTracker settings
	TargetBuildingTracker_->SetUnitClasses( UnitClasses );
}
