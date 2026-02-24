// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/TargetBuildingTracker.h"

#include "Building/Building.h"
#include "Units/Unit.h"

#include "Components/ChooseAttackTargetComponent.h"
#include "Kismet/GameplayStatics.h"

void UTargetBuildingTracker::OnWaveStart()
{
	ScanLevelForBuildings();
}

void UTargetBuildingTracker::OnBuildingDestroyed( ABuilding* building )
{
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
		for ( const auto& [unitClass, _] : TargetBuildings_ )
		{
			TSet<TSubclassOf<ABuilding>> targetClasses;
			if ( const AUnit* cdo = unitClass->GetDefaultObject<AUnit>() )
			{
				if ( const auto* comp = cdo->FindComponentByClass<UChooseAttackTargetComponent>() )
				{
					targetClasses = comp->TargetBuildingClasses();
				}
			}

			for ( const TSubclassOf<ABuilding>& targetClass : targetClasses )
			{
				if ( buildingActor->IsA( targetClass ) )
				{
					if ( ABuilding* building = Cast<ABuilding>( buildingActor ) )
					{
						TargetBuildings_[unitClass].Buildings.Add( building );
						break;
					}
				}
			}
		}
	}
}
