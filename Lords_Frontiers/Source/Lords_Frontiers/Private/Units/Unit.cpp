// Fill out your copyright notice in the Description page of Project Settings.


#include "Lords_Frontiers/Public/Units/Unit.h"


/** (Gregory-hub)
 * Attack someone or something
 * Calls IAttackable::TakeDamage on target */
void AUnit::Attack(TScriptInterface<IAttackable> target)
{
	// Probably should be done with some attack component, because some buildings can attack as well
	// Probably should use some attack manager, because it would be easier to fetch attack info
}

/** (Gregory-hub)
 * Decrease HP */
void AUnit::TakeDamage(float damage)
{
	// Stats.HP -= damage
}

const TObjectPtr<UBehaviorTree>& AUnit::BehaviorTree() const
{
	return BehaviorTree_;
}
