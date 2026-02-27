// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Subsystems/ProjectilePoolSubsystem/ProjectilePoolSubsystem.h"

#include "Projectiles/BaseProjectile.h"

void UProjectilePoolSubsystem::Initialize( FSubsystemCollectionBase& Collection )
{
	Super::Initialize( Collection );
}

void UProjectilePoolSubsystem::Deinitialize()
{
	const int32 TotalPooled = Pools.Num();
	Pools.Empty();
	ActiveCounts.Empty();

	Super::Deinitialize();
}

ABaseProjectile* UProjectilePoolSubsystem::AcquireProjectile( TSubclassOf<ABaseProjectile> projectileClass )
{
	if ( !projectileClass )
	{
		return nullptr;
	}

	FProjectilePool& pool = Pools.FindOrAdd( projectileClass );

	ABaseProjectile* projectile = nullptr;

	while ( pool.Projectiles.Num() > 0 && !projectile )
	{
		projectile = pool.Projectiles.Pop( EAllowShrinking::No );

		if ( !IsValid( projectile ) )
		{
			projectile = nullptr;
		}
	}

	if ( !projectile )
	{
		projectile = CreateNewProjectile( projectileClass );
	}

	if ( projectile )
	{
		ActiveCounts.FindOrAdd( projectileClass )++;
	}
	return projectile;
}

void UProjectilePoolSubsystem::ReturnProjectile( ABaseProjectile* projectile )
{
	if ( !IsValid( projectile ) )
	{
		return;
	}
	projectile->DeactivateToPool();
	TSubclassOf<ABaseProjectile> projectileClass = projectile->GetClass();
	Pools.FindOrAdd( projectileClass ).Projectiles.Add( projectile );

	int32& count = ActiveCounts.FindOrAdd( projectileClass );
	count = FMath::Max( 0, count - 1 );
}

void UProjectilePoolSubsystem::PreWarmPool( TSubclassOf<ABaseProjectile> projectileClass, int32 count )
{
	if ( !projectileClass || count <= 0 )
	{
		return;
	}
	FProjectilePool& pool = Pools.FindOrAdd( projectileClass );
	pool.Projectiles.Reserve( pool.Projectiles.Num() + count );

	int32 successCount = 0;
	for ( int32 i = 0; i < count; ++i )
	{
		if ( ABaseProjectile* proj = CreateNewProjectile( projectileClass ) )
		{
			proj->DeactivateToPool();
			pool.Projectiles.Add( proj );
			successCount++;
		}
	}
}

void UProjectilePoolSubsystem::PreWarmPools( const TArray<FPoolWarmupConfig>& configs )
{
	for ( const FPoolWarmupConfig& config : configs )
	{
		PreWarmPool( config.ProjectileClass, config.Count );
	}
}

int32 UProjectilePoolSubsystem::GetActiveCount( TSubclassOf<ABaseProjectile> projectileClass ) const
{
	const int32* count = ActiveCounts.Find( projectileClass );
	return count ? *count : 0;
}

int32 UProjectilePoolSubsystem::GetPooledCount( TSubclassOf<ABaseProjectile> projectileClass ) const
{
	const FProjectilePool* pool = Pools.Find( projectileClass );
	return pool ? pool->Projectiles.Num() : 0;
}

ABaseProjectile* UProjectilePoolSubsystem::CreateNewProjectile( TSubclassOf<ABaseProjectile> projectileClass )
{
	UWorld* world = GetWorld();
	if ( !world )
	{
		return nullptr;
	}

	FActorSpawnParameters params;

	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	return world->SpawnActor<ABaseProjectile>(
	    projectileClass, FVector( 0.0f, 0.0f, -10000.0f ), FRotator::ZeroRotator, params
	);
}