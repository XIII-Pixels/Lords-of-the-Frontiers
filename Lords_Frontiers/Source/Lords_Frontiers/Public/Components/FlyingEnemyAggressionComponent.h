// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/EnemyAggressionComponent.h"
#include "CoreMinimal.h"

#include "FlyingEnemyAggressionComponent.generated.h"

UCLASS( ClassGroup = ( Unit ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API UFlyingEnemyAggressionComponent : public UEnemyAggressionComponent
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Flying" )
	bool bIgnoreBuildingsOnPath_ = true;

protected:
	virtual FPathConfig BuildPathConfig(
	    const AUnit& unit, const FVector& start, const FVector& goal, float emptyCellTravelTime
	) const override;

	virtual FVector ProjectReachLocation( const FVector& location ) const override
	{
		return FVector( location.X, location.Y, 0.0f );
	}
};