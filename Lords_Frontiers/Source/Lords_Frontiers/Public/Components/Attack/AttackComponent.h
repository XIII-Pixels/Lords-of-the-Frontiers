// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "AttackComponent.generated.h"

class IEntity;
class USceneComponent;

UENUM()
enum class EAttackFilter : uint8
{
	WhatIsOnPath,
	Everything
};

/** (Gregory-hub)
 * Base class for attack components */
UCLASS( Abstract )
class LORDS_FRONTIERS_API UAttackComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	virtual void Attack( TObjectPtr<AActor> hitActor ) PURE_VIRTUAL();

	virtual TObjectPtr<const AActor> AttackTarget() const
	    PURE_VIRTUAL( UAttackComponent::AttackTarget, { return nullptr; } );

	virtual void ActivateSight() PURE_VIRTUAL();
	virtual void DeactivateSight() PURE_VIRTUAL();

	virtual bool DidSeeTargetLastTick() PURE_VIRTUAL( UAttackComponent::DidSeeTargetLastTick, { return false; } );
};
