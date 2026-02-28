// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Interfaces/IMWPoolable.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Baseprojectile.generated.h"

class USphereComponent;


UENUM( BlueprintType )
enum class EProjectileType : uint8
{
	Arrow_Tower,
	Arrow_Enemy,
	Cannonball,
	MagicProjectile,
	Fireball_Enemy,
	Melee_Single,
	Melee_AoE,
	Catapult
};

/**
 * Maxim
 */
UCLASS()
class LORDS_FRONTIERS_API ABaseProjectile : public AActor, public IMWPoolable
{
	GENERATED_BODY()

public:
	ABaseProjectile();

	virtual void ActivateFromPool() override;
	virtual void DeactivateToPool() override;

	virtual void Tick( float deltaTime ) override;

	void InitializeProjectile(
	    AActor* inInstigator, AActor* inTarget, float inDamage, float inSpeed,
	    const FVector& spawnOffset = FVector::ZeroVector, float inSplashRadius = 0.f, float inMaxRange = 0.f
	);

	UPROPERTY( EditDefaultsOnly, Category = "Setting|Projectile" )
	EProjectileType ProjectileType = EProjectileType::Arrow_Tower;

protected:
	UPROPERTY( VisibleAnywhere, Category = "Setting|Projectile" )
	TObjectPtr<USphereComponent> CollisionComp_;

	UPROPERTY()
	TObjectPtr<AActor> Target_;

	float Damage_ = 0.0f;
	float Speed_ = 0.0f;
	bool bIsActive_ = false;
	float MaxRange_ = 0.0f;
	float ArcScale_ = 1.0f;

	UPROPERTY( EditDefaultsOnly, Category = "Setting|Projectile" )
	float MaxLifetime = 5.0f;

	float SplashRadius_ = 0.0f;

	UPROPERTY( EditDefaultsOnly, Category = "Setting|Projectile" )
	float ArcHeight_ = 150.0f;

	UPROPERTY( EditDefaultsOnly, Category = "Setting|Projectile" )
	bool bTrackTarget_ = true;

	FVector StartLocation_;
	FVector TargetLocation_;
	float FlightProgress_ = 0.0f;
	float FlightDuration_ = 0.0f;

	FTimerHandle LifetimeTimerHandle;

	virtual void DealDamage( AActor* hitActor ) const;

	void ReturnToPool();

	void OnLifetimeExpired();

	UFUNCTION()
	void OnCollisionStart(
	    UPrimitiveComponent* overlappedComp, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex,
	    bool bFromSweep, const FHitResult& sweepResult
	);
};
