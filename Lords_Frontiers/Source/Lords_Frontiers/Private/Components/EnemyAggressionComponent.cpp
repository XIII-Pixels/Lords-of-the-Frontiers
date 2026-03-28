// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/EnemyAggressionComponent.h"

#include "AI/Path/Path.h"
#include "AI/Path/PathPointsManager.h"
#include "AI/Path/PathTargetPoint.h"
#include "AI/TargetBuildingTracker.h"
#include "AI/UnitAIManager.h"
#include "Core/CoreManager.h"
#include "Grid/GridManager.h"

#include "Kismet/GameplayStatics.h"

UEnemyAggressionComponent::UEnemyAggressionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UEnemyAggressionComponent::BeginPlay()
{
	Super::BeginPlay();

	if ( const UCoreManager* cm = UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<UCoreManager>() )
	{
		UnitAIManager_ = cm->GetUnitAIManager();
		if ( !UnitAIManager_.IsValid() )
		{
			UE_LOG( LogTemp, Error, TEXT( "UEnemyAggressionComponent::BeginPlay: UnitAIManager_ is not valid" ) );
		}
	}
}

void UEnemyAggressionComponent::TickComponent(
    float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction
)
{
	Super::TickComponent( deltaTime, tickType, thisTickFunction );

	const AUnit* unit = GetOwner<AUnit>();
	if ( !unit )
	{
		UE_LOG( LogTemp, Error, TEXT( "UUnitChooseTargetComponent::TickComponent: owner is not unit" ) );
		return;
	}

	const TWeakObjectPtr<const AActor> target = unit->FollowedTarget();
	if ( target.IsValid() && target->IsA( APathTargetPoint::StaticClass() ) && IsCloseToTarget() )
	{
		FollowNextPathTarget();
	}

	if ( !unit->TargetBuilding().IsValid() || unit->TargetBuilding()->IsDestroyed() )
	{
		FindPathToClosestBuilding();
	}
}

void UEnemyAggressionComponent::FollowNextPathTarget()
{
	AdvancePathPointIndex();
	FollowPath();
}

void UEnemyAggressionComponent::AdvancePathPointIndex()
{
	++PathPointIndex_;
}

void UEnemyAggressionComponent::FollowPath() const
{
	AUnit* unit = GetOwner<AUnit>();
	if ( !unit )
	{
		UE_LOG( LogTemp, Error, TEXT( "UEnemyAggressionComponent::FollowPath: no unit owner" ) );
		return;
	}

	if ( !Path_ )
	{
		UE_LOG( LogTemp, Error, TEXT( "UEnemyAggressionComponent::FollowPath: no valid Path_. Cannot follow path" ) );
		unit->SetFollowedTarget( nullptr );
		return;
	}

	const TArray<FIntPoint>& pathPoints = Path_->GetPoints();
	if ( 0 > PathPointIndex_ || PathPointIndex_ >= pathPoints.Num() )
	{
		if ( unit->TargetBuilding().IsValid() )
		{
			unit->SetFollowedTarget( unit->TargetBuilding() );
		}
		else if ( UnitAIManager_.IsValid() && UnitAIManager_->GoalActor().IsValid() )
		{
			unit->SetFollowedTarget( UnitAIManager_->GoalActor() );
		}
		else
		{
			UE_LOG(
			    LogTemp, Error,
			    TEXT( "UEnemyAggressionComponent::FollowPath: PathPointIndex_ is out of range; failed to get "
			          "UnitAIManager_::GoalActor" )
			);
			unit->SetFollowedTarget( nullptr );
		}
	}
	else if ( UnitAIManager_.IsValid() && UnitAIManager_->GoalActor().IsValid() )
	{
		if ( IsValid( UnitAIManager_->PathPointsManager() ) )
		{
			unit->SetFollowedTarget(
				Cast<AActor>( UnitAIManager_->PathPointsManager()->GetTargetPoint( pathPoints[PathPointIndex_] ) )
			);
		}
	}
}

bool UEnemyAggressionComponent::IsCloseToTarget() const
{
	const AUnit* unit = GetOwner<AUnit>();
	if ( !unit )
	{
		UE_LOG( LogTemp, Error, TEXT( "UEnemyAggressionComponent::IsCloseToTarget: no unit owner" ) );
		return false;
	}

	if ( !unit->FollowedTarget().IsValid() || !UnitAIManager_.IsValid() ||
	     !IsValid( UnitAIManager_->PathPointsManager() ) )
	{
		UE_LOG( LogTemp, Error, TEXT( "UEnemyAggressionComponent::IsCloseToTarget: somethi" ) );
		return false;
	}

	const float distanceSq =
	    FVector::DistSquared( unit->GetActorLocation(), unit->FollowedTarget()->GetActorLocation() );
	const float radius = UnitAIManager_->PathPointsManager()->PointReachRadius();
	const float radiusSq = radius * radius;
	return distanceSq < radiusSq;
}

void UEnemyAggressionComponent::FindPathToClosestBuilding()
{
	AUnit* unit = GetOwner<AUnit>();
	if ( !unit )
	{
		UE_LOG( LogTemp, Error, TEXT( "UEnemyAggressionComponent::FindPathToClosestBuilding: owner is not unit" ) );
		return;
	}

	if ( !UnitAIManager_.IsValid() )
	{
		UE_LOG(
		    LogTemp, Error, TEXT( "UEnemyAggressionComponent::FindPathToClosestBuilding: UnitAIManager is not valid" )
		);
		return;
	}

	unit->SetTargetBuilding( UnitAIManager_->TargetBuildingTracker()->FindClosestBuilding( unit ) );

	const AGridManager* grid = nullptr;
	if ( const UCoreManager* core = UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<UCoreManager>() )
	{
		grid = core->GetGridManager();
	}

	if ( unit->TargetBuilding().IsValid() && grid )
	{
		UPath* path = NewObject<UPath>( unit );
		path->Initialize(
		    { unit->GetActorLocation(), unit->TargetBuilding()->GetTargetLocation(), unit->Stats().AttackDamage(),
		      unit->Stats().AttackCooldown(), grid->GetCellSize() / unit->Stats().MaxSpeed() }
		);
		path->CalculateOrUpdate();
		if ( UnitAIManager_.IsValid() )
		{
			UnitAIManager_->PathPointsManager()->AddPathPoints( *path );
		}

		SetPath( path );
		FollowPath();
	}
}
