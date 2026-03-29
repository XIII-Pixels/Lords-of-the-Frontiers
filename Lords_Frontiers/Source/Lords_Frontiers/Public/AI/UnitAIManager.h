// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Units/Unit.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "UnitAIManager.generated.h"

class APathTargetPoint;
class UTargetBuildingTracker;
class UPathPointsManager;

/* (Gregory-hub)
 * Holds managers and their settings used by AI
 * Needs to be spawned in a level */
UCLASS()
class LORDS_FRONTIERS_API AUnitAIManager : public AActor
{
	GENERATED_BODY()

public:
	AUnitAIManager();

	void OnPreWaveStart();

	UPathPointsManager* PathPointsManager() const
	{
		return PathPointsManager_;
	}

	UTargetBuildingTracker* TargetBuildingTracker() const
	{
		return TargetBuildingTracker_;
	}

	TWeakObjectPtr<const AActor> GoalActor() const
	{
		return GoalActor_;
	}

	TSubclassOf<const APathTargetPoint> PathPointClass() const
	{
		return PathPointClass_;
	}

	float PathPointReachRadius() const
	{
		return PathPointReachRadius_;
	}

	float GroundHeight() const
	{
		return GroundHeight_;
	}

protected:

	virtual void BeginPlay() override;

	void FindGoalActor();

	UPROPERTY( EditAnywhere, Category = "Settings|Path", meta = ( AllowAbstract = "true" ) )
	TSubclassOf<AActor> GoalActorClass_;

	UPROPERTY( EditAnywhere, Category = "Settings|Path" )
	TSubclassOf<APathTargetPoint> PathPointClass_;

	UPROPERTY( EditAnywhere, Category = "Settings|Path" )
	float PathPointReachRadius_ = 100.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|Ground" )
	float GroundHeight_ = 10.0f;

	UPROPERTY()
	TWeakObjectPtr<AActor> GoalActor_;

	UPROPERTY()
	TObjectPtr<UPathPointsManager> PathPointsManager_;

	UPROPERTY()
	TObjectPtr<UTargetBuildingTracker> TargetBuildingTracker_;
};
