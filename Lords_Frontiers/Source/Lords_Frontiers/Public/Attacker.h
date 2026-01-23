// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "Attacker.generated.h"

UINTERFACE( Blueprintable )
class LORDS_FRONTIERS_API UAttacker : public UInterface
{
	GENERATED_BODY()
};

/** (Gregory-hub)
 *  Interface for actors that can attack */
class LORDS_FRONTIERS_API IAttacker
{
	GENERATED_BODY()

public:
	virtual void Attack( TObjectPtr<AActor> hitActor ) = 0;

	virtual TObjectPtr<AActor> EnemyInSight() const = 0;
};
