// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

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
	TWeakObjectPtr<AActor> GetTargetPoint( const FIntPoint& point ) const;

	void Remove( const FIntPoint& point );
	void Empty();

	UPROPERTY( EditDefaultsOnly, Category = "Settings" )
	TSubclassOf<APathTargetPoint> PathTargetPointClass;

private:
	TWeakObjectPtr<AGridManager> Grid_;

	TMap<FIntPoint, TObjectPtr<APathTargetPoint>> PathPoints_;
};
