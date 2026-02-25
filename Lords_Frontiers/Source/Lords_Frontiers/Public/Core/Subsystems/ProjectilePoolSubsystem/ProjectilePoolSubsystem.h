// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Subsystems/WorldSubsystem.h"

#include "CoreMinimal.h"

#include "ProjectilePoolSubsystem.generated.h"

class ABaseProjectile;

USTRUCT( BlueprintType )
struct FProjectilePool
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<ABaseProjectile>> Projectiles;
};

USTRUCT( BlueprintType )
struct FPoolWarmupConfig
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, Category = "Settings|Pool" )
	TSubclassOf<ABaseProjectile> ProjectileClass;

	UPROPERTY( EditAnywhere, Category = "Settings|Pool" )
	int32 Count = 20;
};

/**
 * Maxim
 */
UCLASS()
class LORDS_FRONTIERS_API UProjectilePoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize( FSubsystemCollectionBase& Collection ) override;
	virtual void Deinitialize() override;

	ABaseProjectile* AcquireProjectile( TSubclassOf<ABaseProjectile> ProjectileClass );

	void ReturnProjectile( ABaseProjectile* Projectile );

	void PreWarmPool( TSubclassOf<ABaseProjectile> ProjectileClass, int32 Conut );

	void PreWarmPools( const TArray<FPoolWarmupConfig>& Configs );

	int32 GetActiveCount( TSubclassOf<ABaseProjectile> ProjectileClass ) const;

	int32 GetPooledCount( TSubclassOf<ABaseProjectile> ProjectileClass ) const;

private:
	ABaseProjectile* CreateNewProjectile( TSubclassOf<ABaseProjectile> ProjectileClass );

	UPROPERTY()
	TMap<TSubclassOf<ABaseProjectile>, FProjectilePool> Pools;

	TMap<TSubclassOf<ABaseProjectile>, int32> ActiveCounts;
};
