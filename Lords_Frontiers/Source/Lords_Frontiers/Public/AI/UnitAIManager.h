// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "UnitAIManager.generated.h"

class AUnit;
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

	// Settings

	UPROPERTY( EditAnywhere, Category = "Settings|Path" )
	TWeakObjectPtr<AActor> GoalActor;

	UPROPERTY( EditAnywhere, Category = "Settings|Path" )
	TSubclassOf<APathTargetPoint> PathPointClass;

	UPROPERTY( EditAnywhere, Category = "Settings|Path" )
	float PathPointReachRadius = 200.0f;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|BuildingTracker" )
	TSet<TSubclassOf<AUnit>> UnitClasses;

	// Managers

	UPROPERTY()
	TObjectPtr<UPathPointsManager> PathPointsManager;

	UPROPERTY()
	TObjectPtr<UTargetBuildingTracker> TargetBuildingTracker;

protected:
	virtual void BeginPlay() override;
};
