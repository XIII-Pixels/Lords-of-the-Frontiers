// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Attack/UnitAttackRangedComponent.h"

#include "Units/Unit.h"

void UUnitAttackRangedComponent::BeginPlay()
{
	Super::BeginPlay();

	SetAttackMode();
	ActivateSight();
}

void UUnitAttackRangedComponent::SetAttackMode()
{
	AttackFilter_ = EAttackFilter::WhatIsOnPath;
}

bool UUnitAttackRangedComponent::EnemyIsValid( const AActor* enemyActor ) const
{
	if ( bOnlyAttackTargetBuilding_ )
	{
		if ( !enemyActor )
		{
			return false;
		}

		const AUnit* unit = GetOwner<AUnit>();
		if ( unit && unit->TargetBuilding().Get() != enemyActor )
		{
			return false;
		}
	}

	return Super::EnemyIsValid( enemyActor );
}
