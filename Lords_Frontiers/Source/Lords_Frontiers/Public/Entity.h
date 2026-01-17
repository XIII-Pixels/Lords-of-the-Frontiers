// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EntityStats.h"

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "Entity.generated.h"

enum class ETeam : uint8;

UINTERFACE( Blueprintable )
class LORDS_FRONTIERS_API UEntity : public UInterface
{
	GENERATED_BODY()
};

/** (Gregory-hub)
 *  Interface for all things that can take damage */
class LORDS_FRONTIERS_API IEntity
{
	GENERATED_BODY()

public:
	// Attackers call this on attacked object to deal damage
	virtual void TakeDamage( float damage ) = 0;

	virtual FEntityStats& Stats() = 0;

	virtual ETeam Team() = 0;
};
