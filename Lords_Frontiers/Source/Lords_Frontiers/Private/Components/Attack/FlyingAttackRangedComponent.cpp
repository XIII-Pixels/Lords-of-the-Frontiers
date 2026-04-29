// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Attack/FlyingAttackRangedComponent.h"

#include "Building/Building.h"
#include "Units/Unit.h"

bool UFlyingAttackRangedComponent::IsAttackable( TObjectPtr<AActor> enemyActor ) const
{
	if ( !bOnlyAttackGoalBuilding_ )
	{
		return true;
	}

	const ABuilding* building = Cast<ABuilding>( enemyActor );
	if ( !building )
	{
		return true;
	}

	const AUnit* unit = GetOwner<AUnit>();
	return unit && unit->TargetBuilding().Get() == building;
}