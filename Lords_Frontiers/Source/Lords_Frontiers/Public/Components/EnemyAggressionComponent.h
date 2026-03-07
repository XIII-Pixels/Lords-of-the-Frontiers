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

	const TSet<TSubclassOf<ABuilding>>& TargetBuildingClasses() const
	{
		return TargetBuildingClasses_;
	}

	void SetPath( UPath* path );

	void AdvancePathPointIndex();
	void SetPathPointIndex( int pathPointIndex );

	void FollowPath();

protected:
	virtual void BeginPlay() override;

	void FollowNextPathTarget();

	bool IsCloseToTarget() const;

	void FindPathToClosestBuilding();

	virtual void
	TickComponent( float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction ) override;

	// Priority list of building classes
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings" )
	TSet<TSubclassOf<ABuilding>> TargetBuildingClasses_;

	UPROPERTY()
	TWeakObjectPtr<ABuilding> TargetBuilding_;

	UPROPERTY()
	TWeakObjectPtr<AUnitAIManager> UnitAIManager_;

	UPROPERTY()
	TObjectPtr<UPath> Path_;

	int PathPointIndex_ = -1;
};
