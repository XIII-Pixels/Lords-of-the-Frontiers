// Fill out your copyright notice in the Description page of Project Settings.


#include "Lords_Frontiers/Public/Unit.h"


// Sets default values
AUnit::AUnit()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called to bind functionality to input
void AUnit::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

bool AUnit::Attack(TScriptInterface<IAttackable> target)
{
	// Probably should be done with some attack component, because some buildings can attack as well
	// Probably should use some attack manager, because it would be easier to fetch attack info
	return false;
}

bool AUnit::TakeDamage(float damage)
{
	return false;
}
