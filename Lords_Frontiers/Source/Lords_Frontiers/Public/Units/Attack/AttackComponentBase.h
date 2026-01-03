// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackComponentBase.generated.h"

/** (Gregory-hub)
* Base class for attack components */
UCLASS( Abstract )
class LORDS_FRONTIERS_API UAttackComponentBase : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void Attack(TObjectPtr<AActor> hitActor) PURE_VIRTUAL();

	virtual TObjectPtr<AActor> EnemyInSight() const PURE_VIRTUAL(
		UAttackComponentBase::EnemyInSight,
		{ return nullptr; } );
};
