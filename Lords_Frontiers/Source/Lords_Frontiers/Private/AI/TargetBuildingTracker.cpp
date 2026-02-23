// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/TargetBuildingTracker.h"

void UTargetBuildingTracker::OnWaveStart()
{
	ScanLevelForBuildings();
}

void UTargetBuildingTracker::ScanLevelForBuildings()
{
	UWorld* world = GetWorld();
	if ( !world )
	{
		UE_LOG( LogTemp, Error, TEXT( "UChooseAttackTargetComponent: world not found" ) );
		return;
	}

	TArray<AActor*> buildingActors;
	UGameplayStatics::GetAllActorsOfClass( world, ABuilding::StaticClass(), buildingActors );

	for ( AActor* buildingActor : buildingActors )
	{
		for ( const TSubclassOf<ABuilding> priorityClass : PriorityClasses_ )
		{
			if ( buildingActor->IsA( priorityClass ) )
			{
				if ( ABuilding* building = Cast<ABuilding>( buildingActor ) )
				{
					PriorityBuildings_.Add( building );
					break;
				}
			}
		}
	}
}
