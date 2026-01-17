// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "AttackComponent.generated.h"

class IEntity;
class USceneComponent;

/** (Gregory-hub)
 * Base class for attack components */
UCLASS( Abstract, ClassGroup = ( Attack ) )
class LORDS_FRONTIERS_API UAttackComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	virtual void Attack( TObjectPtr<AActor> hitActor ) PURE_VIRTUAL();

	virtual TObjectPtr<AActor> EnemyInSight() const
	    PURE_VIRTUAL( UAttackComponentBase::EnemyInSight, { return nullptr; } );

	virtual void ActivateSight() PURE_VIRTUAL();

	virtual void DeactivateSight() PURE_VIRTUAL();

protected:
	virtual void OnRegister() override;

	bool OwnerIsValid() const;

	// Pointer to Owner of type IEntity
	IEntity* OwnerEntity_;

private:
	bool bOwnerIsValid_ = true;
};
