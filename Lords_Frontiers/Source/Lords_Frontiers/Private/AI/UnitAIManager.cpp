// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/UnitAIManager.h"

#include "AI/Path/PathPointsManager.h"
#include "AI/TargetBuildingTracker.h"

#include "Kismet/GameplayStatics.h"

AUnitAIManager::AUnitAIManager()
{
}

void AUnitAIManager::BeginPlay()
{
	Super::BeginPlay();

	PathPointsManager_ = NewObject<UPathPointsManager>( this );
	TargetBuildingTracker_ = NewObject<UTargetBuildingTracker>( this );

	FindGoalActor();

	// PathPointsManager settings
	PathPointsManager_->SetGoalActor( GoalActor_ );
	PathPointsManager_->SetPathTargetPointClass( PathPointClass_ );
	PathPointsManager_->SetPointReachRadius( PathPointReachRadius_ );

	// TargetBuildingTracker settings
	TargetBuildingTracker_->Initialize();
}

void AUnitAIManager::FindGoalActor()
{
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass( GetWorld(), GoalActorClass_, foundActors );

	if ( foundActors.Num() == 0 )
	{
		UE_LOG( LogTemp, Error, TEXT( "AUnitAIManager::FindGoalActor: no goal actors found" ) );
		return;
	}

	if ( foundActors.Num() != 1 )
	{
		UE_LOG( LogTemp, Error, TEXT( "AUnitAIManager::FindGoalActor: more than 1 goal actor was found" ) );
	}

	GoalActor_ = foundActors[0];
}

void AUnitAIManager::OnPreWaveStart()
{
	if ( IsValid( TargetBuildingTracker_ ) )
	{
		TargetBuildingTracker_->ScanLevelForBuildings();
	}
}
