// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AttackComponentBase.h"
#include "CoreMinimal.h"

#include "AttackRangedComponent.generated.h"

class AUnit;
class USphereComponent;
class AProjectile;

/** (Gregory-hub)
 * Makes actor attack enemy actors in sight */
UCLASS( ClassGroup = ( Unit ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API UAttackRangedComponent : public UAttackComponentBase
{
	GENERATED_BODY()

public:
	UAttackRangedComponent();

	// Launch projectile
	virtual void Attack( TObjectPtr<AActor> hitActor ) override;

	virtual TObjectPtr<AActor> EnemyInSight() const override;

protected:
	virtual void OnRegister() override;

	virtual void BeginPlay() override;

	// Look forward around at given time intervals
	void SightTick();

	void Look();

	bool CanSeeEnemy( TObjectPtr<AActor> actor ) const;

	UPROPERTY( EditAnywhere, Category = "Settings|Attack" )
	float LookForwardTimeInterval_ = 0.2f;

	UPROPERTY( VisibleAnywhere, Category = "Settings|Attack" )
	TObjectPtr<AActor> EnemyInSight_;

	UPROPERTY( EditAnywhere, Category = "Settings|Attack" )
	bool bCanAttackBackward_ = true;

	UPROPERTY( EditAnywhere, Category = "Settings|Attack" )
	TSubclassOf<AProjectile> ProjectileClass_;

	UPROPERTY( EditAnywhere, Category = "Settings|Attack" )
	float ProjectileSpeed_ = 1500.0f;

	UPROPERTY()
	TObjectPtr<USphereComponent> SightSphere_;

	UPROPERTY()
	TObjectPtr<AUnit> Unit_;

	FTimerHandle SightTimerHandle_;
};
