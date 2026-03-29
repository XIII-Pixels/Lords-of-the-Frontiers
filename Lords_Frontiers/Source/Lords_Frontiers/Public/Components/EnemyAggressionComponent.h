// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Building/Building.h"

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "EnemyAggressionComponent.generated.h"

class UPath;
class AUnitAIManager;
class UTargetBuildingTracker;
class ABuilding;

/** (Gregory-hub)
 * Finds closest building that unit should attack */
UCLASS( ClassGroup = ( Unit ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API UEnemyAggressionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyAggressionComponent();

	void FollowNextPathTarget();

	void FollowPath() const;

	const TSet<TSoftClassPtr<ABuilding>>& TargetBuildingClasses() const
	{
		return TargetBuildingClasses_;
	}

	void SetPathPointIndex( int pathPointIndex )
	{
		PathPointIndex_ = pathPointIndex;
	}

	UPath* Path() const
	{
		return Path_;	
	}

	void SetPath( UPath* path )
	{
		Path_ = path;
		PathPointIndex_ = 0;
	}

protected:
	virtual void BeginPlay() override;

	void AdvancePathPointIndex();

	bool IsCloseToTarget() const;

	void FindPathToClosestBuilding();

	virtual void
	TickComponent( float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction ) override;

	// Set of building classes (not ordered). Unit attacks closest building from set
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = ( AllowAbstract = "true" ) )
	TSet<TSoftClassPtr<ABuilding>> TargetBuildingClasses_;

	UPROPERTY()
	TWeakObjectPtr<AUnitAIManager> UnitAIManager_;

	UPROPERTY()
	TObjectPtr<UPath> Path_;

	int PathPointIndex_ = -1;
};
