// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Attackable.generated.h"

enum class ETeam : uint8;


UINTERFACE(Blueprintable)
class LORDS_FRONTIERS_API UAttackable : public UInterface
{
	GENERATED_BODY()
};

/** (Gregory-hub)
 *  Interface for all things that can take damage */
class LORDS_FRONTIERS_API IAttackable
{
	GENERATED_BODY()

public:
	// Attackers call this on attacked object to deal damage
	virtual void TakeDamage(float damage) = 0;

	virtual ETeam Team() = 0;
};
