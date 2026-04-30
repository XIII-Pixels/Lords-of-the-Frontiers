// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Attack/BuildingAttackRangedComponent.h"

#include "Units/Unit.h"

void UBuildingAttackRangedComponent::BeginPlay()
{
	Super::BeginPlay();

	SetAttackMode();
	ActivateSight();
}

void UBuildingAttackRangedComponent::SetAttackMode()
{
	AttackFilter_ = EAttackFilter::Everything;
}

bool UBuildingAttackRangedComponent::EnemyIsValid( const AActor* enemyActor ) const
{
	if ( !enemyActor )
	{
		return false;
	}

	for ( const auto& ignoredClass : IgnoredUnits_ )
	{
		if ( enemyActor->IsA( ignoredClass ) )
		{
			return false;
		}
	}

	return Super::EnemyIsValid( enemyActor );
}
