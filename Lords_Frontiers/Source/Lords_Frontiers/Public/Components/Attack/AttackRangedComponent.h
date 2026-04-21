// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AttackComponent.h"
#include "EntityStats.h"
#include "Attacker.h"

#include "CoreMinimal.h"

#include "AttackRangedComponent.generated.h"

class IEntity;
class USphereComponent;
class ABaseProjectile;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnAttackFired, AActor*, Target );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnAttackTargetChanged, AActor*, OldTarget, AActor*, NewTarget );

/** (Gregory-hub)
 * Makes actor attack enemy actors in sight */
UCLASS( meta = ( BlueprintSpawnableComponent ), ClassGroup = ( Attack ) )
class LORDS_FRONTIERS_API UAttackRangedComponent : public UAttackComponent
{
	GENERATED_BODY()

public:
	UAttackRangedComponent();

	// Launch projectile
	virtual void Attack( TObjectPtr<AActor> hitActor ) override;

	virtual TObjectPtr<const AActor> AttackTarget() const override
	{
		if ( const IAttacker* attacker = GetOwner<IAttacker>() )
		{
			return attacker->AttackTarget().Get();
		}
		return nullptr;
	}

	virtual void ActivateSight() override;
	virtual void DeactivateSight() override;

	virtual bool DidSeeTargetLastTick() override;

	UPROPERTY( BlueprintAssignable, Category = "Attack|Events" )
	FOnAttackFired OnAttackFired;

	UPROPERTY( BlueprintAssignable, Category = "Attack|Events" )
	FOnAttackTargetChanged OnAttackTargetChanged;

protected:
	virtual void OnRegister() override;

	virtual void BeginPlay() override;

	// Runs at given time intervals
	void LookTick();

	// Look around
	void Look();

	void ChooseAttackMode();

	bool CanSeeEnemy( TObjectPtr<AActor> enemyActor ) const;

	void FireSingleProjectile( TWeakObjectPtr<AActor> target ) const;

	void FireNextBurstShot();

	TArray<TObjectPtr<AActor>> FindNeighborTargets( int32 count ) const;

	UPROPERTY( EditAnywhere, Category = "Settings|Attack" )
	float LookForwardTimeInterval_ = 0.2f;

	UPROPERTY( EditAnywhere, Category = "Settings|Attack" )
	bool bCanAttackBackward_ = true;

	UPROPERTY( EditAnywhere, Category = "Settings|Attack" )
	TSubclassOf<ABaseProjectile> ProjectileClass_;

	UPROPERTY( EditAnywhere, Category = "Settings|Attack" )
	float ProjectileSpeed_ = 1500.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|Attack" )
	FVector ProjectileSpawnPosition_;

	UPROPERTY( EditAnywhere, Category = "Settings|Attack" )
	bool bProjectileTracksTarget_ = true;

	TObjectPtr<USphereComponent> SightSphere_;

	FTimerHandle SightTimerHandle_;

	EAttackFilter AttackFilter_ = EAttackFilter::Everything;

	int32 CurrentBurstIndex_ = 0;
	bool bBurstInProgress_ = false;

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> BurstTargets_;

	FTimerHandle BurstTimerHandle_;

	bool bDidSeeTarget_ = false;
};
