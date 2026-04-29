// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/FlyingEnemyAggressionComponent.h"

#include "AI/Path/Path.h"

FPathConfig UFlyingEnemyAggressionComponent::BuildPathConfig(
    const AUnit& unit, const FVector& start, const FVector& goal, float emptyCellTravelTime
) const
{
	if ( !bIgnoreBuildingsOnPath_ )
	{
		return Super::BuildPathConfig( unit, start, goal, emptyCellTravelTime );
	}

	FPathConfig config( start, goal, 0, 0.0f, 1.0f );
	config.bIgnoreObstacles = true;
	return config;
}
