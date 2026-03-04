// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Projectiles/BaseProjectile.h"

#include "CoreMinimal.h"

#include "AoEProjectile.generated.h"

/**
 * Maxim
 */
UCLASS()
class LORDS_FRONTIERS_API AAoEProjectile : public ABaseProjectile
{
	GENERATED_BODY()

public:
	AAoEProjectile();

protected:
	UPROPERTY( EditDefaultsOnly, Category = "Settings|AoE" )
	float AoERadius = 200.0f;

	virtual void DealDamage( AActor* HitActor ) const override;
};
