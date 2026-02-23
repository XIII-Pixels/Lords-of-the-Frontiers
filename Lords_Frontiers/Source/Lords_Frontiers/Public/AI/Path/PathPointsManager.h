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

	void SetGrid( TWeakObjectPtr<AGridManager> grid );
	void AddPathPoints( const UPath& path );
	TWeakObjectPtr<APathTargetPoint> GetTargetPoint( const FIntPoint& point ) const;

	void Remove( const FIntPoint& point );
	void Empty();

	void ShowAll();
	void HideAll();

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
};
