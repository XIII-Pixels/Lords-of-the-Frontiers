// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/TargetBuildingTracker.h"

#include "AI/UnitAIManager.h"
#include "Building/Building.h"
#include "Core/CoreManager.h"
#include "Units/Unit.h"

#include "Components/EnemyAggressionComponent.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Kismet/GameplayStatics.h"

void UTargetBuildingTracker::Initialize()
{
	FindUnitClasses();
}

void UTargetBuildingTracker::ScanLevelForBuildings()
{
	const UWorld* world = GetWorld();
	if ( !world )
	{
		UE_LOG( LogTemp, Error, TEXT( "UTargetBuildingTracker::ScanLevelForBuildings: world not found" ) );
		return;
	}

	TArray<AActor*> buildingActors;
	UGameplayStatics::GetAllActorsOfClass( world, ABuilding::StaticClass(), buildingActors );

	// Set goal actor to all unit classes
	if ( const UCoreManager* cm = UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<UCoreManager>() )
	{
		if ( const AUnitAIManager* aiManager = cm->GetUnitAIManager() )
		{
			TWeakObjectPtr<const AActor> goalActor = aiManager->GoalActor();
			if ( goalActor.IsValid() )
			{
				for ( const auto& [unitClass, _] : TargetBuildings_ )
				{
					if ( const ABuilding* building = Cast<ABuilding>( goalActor.Get() ) )
					{
						TargetBuildings_[unitClass].Buildings.Add( building );
					}
				}
			}
		}
	}

	// Find buildings for each unit class
	for ( AActor* buildingActor : buildingActors )
	{
		for ( const auto& [unitClass, _] : TargetBuildings_ )
		{
			if ( BuildingIsUnitTarget( buildingActor, unitClass ) )
			{
				if ( const ABuilding* building = Cast<ABuilding>( buildingActor ) )
				{
					TargetBuildings_[unitClass].Buildings.Add( building );
				}
			}
		}
	}
}

void UTargetBuildingTracker::FindUnitClasses()
{
	TArray<UClass*> outClasses;
	GetDerivedClasses( AUnit::StaticClass(), outClasses, true );

	for ( auto* c : outClasses )
	{
		TargetBuildings_.FindOrAdd( c );
	}
}

bool UTargetBuildingTracker::BuildingIsUnitTarget( const AActor* buildingActor, const TSubclassOf<AUnit>& unitClass )
    const
{
	TSet<TSoftClassPtr<ABuilding>> targetClasses;
	if ( UBlueprintGeneratedClass* bpClass = Cast<UBlueprintGeneratedClass>( unitClass ) )
	{
		if ( bpClass || bpClass->SimpleConstructionScript )
		{
			TArray<USCS_Node*> nodes = bpClass->SimpleConstructionScript->GetAllNodes();
			for ( const USCS_Node* node : nodes )
			{
				if ( node->ComponentClass->IsChildOf( UEnemyAggressionComponent::StaticClass() ) )
				{
					UActorComponent* found = node->GetActualComponentTemplate( bpClass );
					const UEnemyAggressionComponent* component = Cast<UEnemyAggressionComponent>( found );
					targetClasses = component->TargetBuildingClasses();
				}
			}
		}
	}

	for ( const TSoftClassPtr<ABuilding>& targetClass : targetClasses )
	{
		if ( buildingActor->IsA( targetClass.Get() ) )
		{
			return true;
		}
	}

	return false;
}

TWeakObjectPtr<const ABuilding> UTargetBuildingTracker::GetDefaultTargetBuilding() const
{
	if ( const UCoreManager* cm = UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<UCoreManager>() )
	{
		if ( const AUnitAIManager* aiManager = cm->GetUnitAIManager() )
		{
			return Cast<const ABuilding>( aiManager->GoalActor().Get() );
		}
	}

	UE_LOG(
	    LogTemp, Error,
	    TEXT( "UTargetBuildingTracker::GetDefaultTargetBuilding: failed to return default building. Returning nullptr" )
	);
	return nullptr;
}

TWeakObjectPtr<const ABuilding> UTargetBuildingTracker::FindClosestBuilding( const AUnit* unit ) const
{
	if ( !IsValid( unit ) )
	{
		UE_LOG(
		    LogTemp, Warning, TEXT( "UTargetBuildingTracker::FindClosestBuildingFor: invalid AUnit is passed as arg" )
		);
		return GetDefaultTargetBuilding();
	}

	TWeakObjectPtr<const ABuilding> closest = nullptr;
	float minDist = FLT_MAX;

	// Tip for possible future optimization:
	// Cache by grid square can be added (to avoid traversing through all buildings for each unit)
	// Tip for further optimization: add same cache value to neighbour cells (warning: changes unit behavior)
	const TSet<TWeakObjectPtr<const ABuilding>>& buildings = TargetBuildings_.Contains( unit->GetClass() )
	                                                             ? TargetBuildings_[unit->GetClass()].Buildings
	                                                             : TSet<TWeakObjectPtr<const ABuilding>>();
	for ( const TWeakObjectPtr<const ABuilding> building : buildings )
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
