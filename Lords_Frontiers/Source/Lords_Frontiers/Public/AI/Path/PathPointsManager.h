// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "PathPointsManager.generated.h"

class ASplinePointConnector;
class AUnit;
class AGridManager;
class UPath;
class APathTargetPoint;

USTRUCT()
struct FPointsOnCell
{
	GENERATED_BODY()

	const TMap<TSubclassOf<AUnit>, TObjectPtr<APathTargetPoint>>& Points() const
	{
		return Points_;
	}

	TMap<TSubclassOf<AUnit>, TObjectPtr<APathTargetPoint>>& Points()
	{
		return Points_;
	}

private:
	TMap<TSubclassOf<AUnit>, TObjectPtr<APathTargetPoint>> Points_;
};

/** (Gregory-hub)
 * Class for storing and retrieving path points that exist in the world and can be followed */
UCLASS()
class LORDS_FRONTIERS_API UPathPointsManager : public UObject
{
	GENERATED_BODY()

public:
	void GetAccessToGrid();

	void CreateAndRegisterPathPoints( const UPath& path, TSubclassOf<AUnit> unitClass );
	void RegisterPoint( const FIntPoint& point, APathTargetPoint* pathPoint, TSubclassOf<AUnit> unitClass );

	TWeakObjectPtr<APathTargetPoint> GetTargetPoint( const FIntPoint& point, TSubclassOf<AUnit> unitClass ) const;

	// Remove point by value
	void ReleasePathPoint( const FIntPoint& point, TSubclassOf<AUnit> unitClass );
	// Remove points in the path
	void ReleasePath( const UPath* path, TSubclassOf<AUnit> unitClass );
	// Remove all points
	void Empty();

	// Show all points
	void ShowAll();
	// Hide all points
	void HideAll();

	/** Make point mesh visible
	 * @param point - point
	 * @param buildingAware - if true points on cells with buildings are not visible */
	void ShowPoint( APathTargetPoint* point, bool buildingAware = true ) const;
	// Make point mesh invisible
	void HidePoint( APathTargetPoint* point ) const;

	// True if actor is on one of the cells that path steps on
	// Path destination is on path as well
	bool ActorIsOnPath( const AActor* enemyActor, const UPath* path ) const;

	void SetGoalActor( const TWeakObjectPtr<AActor>& goalActor )
	{
		GoalActor_ = goalActor;
	}

	void SetPointReachRadius( float pointReachRadius )
	{
		PointReachRadius_ = pointReachRadius;
	}

	float PointReachRadius() const
	{
		return PointReachRadius_;
	}

private:
	TWeakObjectPtr<APathTargetPoint>
	SpawnPoint( const FIntPoint& point, TSubclassOf<APathTargetPoint> pathPointClass ) const;

	UPROPERTY( EditAnywhere )
	TWeakObjectPtr<AGridManager> Grid_;

	UPROPERTY( EditAnywhere )
	TWeakObjectPtr<AActor> GoalActor_;

	UPROPERTY( EditAnywhere )
	float PointReachRadius_ = 100.0f;

	UPROPERTY()
	TMap<FIntPoint, FPointsOnCell> PathPoints_;

	bool bPointsVisible_ = false;
};
