// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/UnitAIManager.h"

#include "AI/Path/PathPointsManager.h"
#include "AI/TargetBuildingTracker.h"

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

void AUnitAIManager::BeginPlay()
{
	Super::BeginPlay();

	PathPointsManager_ = NewObject<UPathPointsManager>( this );
	TargetBuildingTracker_ = NewObject<UTargetBuildingTracker>( this );

	// PathPointsManager settings
	PathPointsManager_->SetGoalActor( GoalActor );
	PathPointsManager_->SetPathTargetPointClass( PathPointClass );
	PathPointsManager_->SetPointReachRadius( PathPointReachRadius );

	// TargetBuildingTracker settings
	TargetBuildingTracker_->SetUnitClasses( UnitClasses );
}
