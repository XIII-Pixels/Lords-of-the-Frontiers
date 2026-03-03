// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/UnitChooseTargetComponent.h"

#include "AI/Path/Path.h"
#include "AI/Path/PathPointsManager.h"
#include "AI/TargetBuildingTracker.h"
#include "AI/UnitAIManager.h"
#include "Core/CoreManager.h"
#include "Grid/GridManager.h"

#include "Kismet/GameplayStatics.h"

UUnitChooseTargetComponent::UUnitChooseTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UUnitChooseTargetComponent::BeginPlay()
{
	Super::BeginPlay();

	if ( const UCoreManager* cm = UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<UCoreManager>() )
	{
		UnitAIManager_ = cm->GetUnitAIManager();
	}
}

void UUnitChooseTargetComponent::TickComponent(
    float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction
)
{
	Super::TickComponent( deltaTime, tickType, thisTickFunction );

	AUnit* unit = Cast<AUnit>( GetOwner() );
	if ( !unit )
	{
		UE_LOG( LogTemp, Error, TEXT( "UUnitChooseTargetComponent::TickComponent: owner is not unit" ) );
		return;
	}

	if ( !TargetBuilding_.IsValid() || TargetBuilding_->IsDestroyed() )
	{
		TargetBuilding_ = UnitAIManager_->TargetBuildingTracker()->FindClosestBuilding( unit );

		const AGridManager* grid = nullptr;
		if ( const UCoreManager* core = UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<UCoreManager>() )
		{
			grid = core->GetGridManager();
		}

		if ( TargetBuilding_.IsValid() && grid )
		{
			UPath* path = NewObject<UPath>( unit );
			path->Initialize(
			    { unit->GetActorLocation(), TargetBuilding_->GetTargetLocation(), unit->Stats().AttackDamage(),
			      unit->Stats().AttackCooldown(), grid->GetCellSize() / unit->Stats().MaxSpeed() }
			);
			path->CalculateOrUpdate();
			if ( UnitAIManager_.IsValid() )
			{
				UnitAIManager_->PathPointsManager()->AddPathPoints( *path );
			}

			unit->SetPath( path );
			unit->FollowPath();
		}
	}
}
