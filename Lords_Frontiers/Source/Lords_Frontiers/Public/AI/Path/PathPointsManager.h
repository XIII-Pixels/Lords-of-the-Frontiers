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
class LORDS_FRONTIERS_API UPathPointsManager : public UObject
{
	GENERATED_BODY()

public:
	virtual void PostInitProperties() override;

	void InitializeGrid();

	void AddPathPoints( const UPath& path );
	TWeakObjectPtr<APathTargetPoint> GetTargetPoint( const FIntPoint& point ) const;

	// Remove point by value
	void Remove( const FIntPoint& point );
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

	void SetGoalActor( const TWeakObjectPtr<AActor>& goalActor )
	{
		GoalActor_ = goalActor;
	}

	void SetPathTargetPointClass( const TSubclassOf<APathTargetPoint>& pathTargetPointClass )
	{
		PathTargetPointClass_ = pathTargetPointClass;
	}

	void SetPointReachRadius( float pointReachRadius )
	{
		PointReachRadius_ = pointReachRadius;
	}

	void SetPathPoints( const TMap<FIntPoint, TObjectPtr<APathTargetPoint>>& pathPoints )
	{
		PathPoints_ = pathPoints;
	}

	float PointReachRadius() const
	{
		return PointReachRadius_;
	}

private:
	UPROPERTY( EditAnywhere )
	TWeakObjectPtr<AGridManager> Grid_;

	UPROPERTY( EditAnywhere )
	TWeakObjectPtr<AActor> GoalActor_;

	UPROPERTY( EditAnywhere )
	TSubclassOf<APathTargetPoint> PathTargetPointClass_;

	UPROPERTY( EditAnywhere )
	float PointReachRadius_ = 100.0f;

	UPROPERTY()
	TMap<FIntPoint, TObjectPtr<APathTargetPoint>> PathPoints_;

	bool bPointsVisible_ = false;
};
