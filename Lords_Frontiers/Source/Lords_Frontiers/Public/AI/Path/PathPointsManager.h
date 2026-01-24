// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "PathPointsManager.generated.h"

class AGridManager;
class UPath;
class APathTargetPoint;

/** (Gregory-hub)
 * Class for storing and retrieving path points that exist in the world and can be followed */
UCLASS()
class LORDS_FRONTIERS_API APathPointsManager : public AActor
{
	GENERATED_BODY()

public:
	virtual void PostInitProperties() override;

	void SetGrid( TWeakObjectPtr<AGridManager> grid );
	void AddPathPoints( const UPath& path );
	TWeakObjectPtr<AActor> GetTargetPoint( const FIntPoint& point ) const;

	void Remove( const FIntPoint& point );
	void Empty();

	UPROPERTY( EditAnywhere, Category = "Settings" )
	TWeakObjectPtr<AGridManager> Grid;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	TWeakObjectPtr<AActor> GoalActor;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	TSubclassOf<APathTargetPoint> PathTargetPointClass;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	float PointReachRadius = 100.0f;

private:
	TMap<FIntPoint, TObjectPtr<APathTargetPoint>> PathPoints_;
};
