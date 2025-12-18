// Fill out your copyright notice in the Description page of Project Settings.

#include "Units/BehaviorTreeNodes/Decorators/DoesSeeAttackable.h"

#include "Units/Unit.h"

UDoesSeeAttackable::UDoesSeeAttackable()
{
	NodeName = "Does See Attackable Ahead";
}

bool UDoesSeeAttackable::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	auto unit = Cast<AUnit>( OwnerComp.GetOwner() );
	if ( !unit )
	{
		return false;
	}

	return true;
}
