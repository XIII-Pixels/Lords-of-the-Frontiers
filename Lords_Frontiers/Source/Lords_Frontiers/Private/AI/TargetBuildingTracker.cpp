// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/TargetBuildingTracker.h"

#include "AI/UnitAIManager.h"
#include "Building/Building.h"
#include "Core/CoreManager.h"
#include "Units/Unit.h"

#include "Components/UnitChooseTargetComponent.h"
#include "Kismet/GameplayStatics.h"

void UTargetBuildingTracker::OnBuildingDestroyed( ABuilding* building )
{
}

void UTargetBuildingTracker::ScanLevelForBuildings()
{
	const UWorld* world = GetWorld();
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
			if ( BuildingIsUnitTarget( buildingActor, unitClass ) )
			{
				if ( ABuilding* building = Cast<ABuilding>( buildingActor ) )
				{
					TargetBuildings_[unitClass].Buildings.Add( building );
				}
			}
		}
	}
}

bool UTargetBuildingTracker::BuildingIsUnitTarget( const AActor* buildingActor, const TSubclassOf<AUnit>& unitClass ) const
{
	TSet<TSubclassOf<ABuilding>> targetClasses;
	if ( const AUnit* cdo = unitClass->GetDefaultObject<AUnit>() )
	{
		if ( const auto* comp = cdo->FindComponentByClass<UUnitChooseTargetComponent>() )
		{
			targetClasses = comp->TargetBuildingClasses();
		}
	}

	for ( const TSubclassOf<ABuilding>& targetClass : targetClasses )
	{
		if ( buildingActor->IsA( targetClass ) )
		{
			return true;
		}
	}

	return false;
}

TWeakObjectPtr<ABuilding> UTargetBuildingTracker::GetDefaultTargetBuilding() const
{
	if ( const UCoreManager* cm = UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<UCoreManager>() )
	{
		if ( const AUnitAIManager* aiManager = cm->GetUnitAIManager() )
		{
			return Cast<ABuilding>( aiManager->GoalActor );
		}
	}
	return nullptr;
}

TWeakObjectPtr<ABuilding> UTargetBuildingTracker::FindClosestBuilding( const AUnit* unit ) const
{
	if ( !IsValid( unit ) )
	{
		UE_LOG(
		    LogTemp, Warning, TEXT( "UTargetBuildingTracker::FindClosestBuildingFor: invalid AUnit is passed as arg" )
		);
		return GetDefaultTargetBuilding();
	}

	TWeakObjectPtr<ABuilding> closest = nullptr;
	float minDist = FLT_MAX;

	// Tip for possible future optimization:
	// Cache by grid square can be added (to avoid traversing through all buildings for each unit)
	// Tip for further optimization: add same cache value to neighbour cells (warning: changes unit behavior)
	for ( const TWeakObjectPtr<ABuilding> building : TargetBuildings_[unit->GetClass()].Buildings )
	{
		if ( building.IsValid() && !building->IsDestroyed() )
		{
			const float dist = FVector::Distance( unit->GetActorLocation(), building->GetActorLocation() );
			if ( dist < minDist )
			{
				minDist = dist;
				closest = building;
			}
		}
	}

	return closest != nullptr ? closest : GetDefaultTargetBuilding();
}
