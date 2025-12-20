// Fill out your copyright notice in the Description page of Project Settings.

#include "Units/BehaviorTreeNodes/Decorators/DoesSeeEnemyDecorator.h"

#include "Units/Unit.h"

UDoesSeeEnemyDecorator::UDoesSeeEnemyDecorator()
{
	NodeName = "Does See Enemy";
}

bool UDoesSeeEnemyDecorator::CalculateRawConditionValue(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory) const
{
	auto unit = Cast<AUnit>( Cast<AController>( ownerComp.GetOwner() )->GetPawn() );
	if ( !unit )
	{
		return false;
	}

	if ( unit->EnemyInSight() != nullptr )
	{
		return true;
	}
	return false;
}
