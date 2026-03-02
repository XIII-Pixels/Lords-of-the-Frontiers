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

	const UPathPointsManager* PathPointsManager();

	const UTargetBuildingTracker* TargetBuildingTracker();

	UPROPERTY( EditAnywhere, Category = "Settings|Path" )
	TWeakObjectPtr<AActor> GoalActor;

	UPROPERTY( EditAnywhere, Category = "Settings|Path" )
	TSubclassOf<APathTargetPoint> PathPointClass;

	UPROPERTY( EditAnywhere, Category = "Settings|Path" )
	float PathPointReachRadius = 200.0f;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|BuildingTracker" )
	TSet<TSubclassOf<AUnit>> UnitClasses;

protected:
	UPROPERTY()
	TObjectPtr<UPathPointsManager> PathPointsManager_;

	UPROPERTY()
	TObjectPtr<UTargetBuildingTracker> TargetBuildingTracker_;

	virtual void BeginPlay() override;
};
