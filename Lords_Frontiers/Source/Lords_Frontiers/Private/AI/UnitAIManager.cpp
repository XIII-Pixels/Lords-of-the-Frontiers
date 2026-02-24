// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/UnitAIManager.h"

#include "AI/Path/PathPointsManager.h"
#include "AI/TargetBuildingTracker.h"
#include "Grid/GridManager.h"

#include "Kismet/GameplayStatics.h"

AUnitAIManager::AUnitAIManager()
{
	PathPointsManager = NewObject<UPathPointsManager>();
	TargetBuildingTracker = NewObject<UTargetBuildingTracker>();
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

	AGridManager* gridManager =
	    Cast<AGridManager>( UGameplayStatics::GetActorOfClass( world, AGridManager::StaticClass() ) );

	// PathPointsManager settings

	if ( gridManager )
	{
		PathPointsManager->SetGrid( gridManager );
	}
	else
	{
		UE_LOG( LogTemp, Error, TEXT( "AUnitAIManager: grid manager is not found" ) );
	}

	PathPointsManager->SetGoalActor( GoalActor );
	PathPointsManager->SetPathTargetPointClass( PathPointClass );
	PathPointsManager->SetPointReachRadius( PathPointReachRadius );

	// TargetBuildingTracker settings
	TargetBuildingTracker->SetUnitClasses( UnitClasses );
}
