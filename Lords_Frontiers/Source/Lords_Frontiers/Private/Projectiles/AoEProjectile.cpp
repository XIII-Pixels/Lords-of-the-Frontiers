// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectiles/AoEProjectile.h"

#include "Entity.h"
#include "Utilities/TraceChannelMappings.h"
#include "Engine/OverlapResult.h"
#include "Components/SphereComponent.h"

AAoEProjectile::AAoEProjectile()
{
	ProjectileType = EProjectileType::Fireball_Enemy;
}

void AAoEProjectile::DealDamage( AActor* HitActor ) const
{
	TArray<FOverlapResult> overlaps;
	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor( this );
	queryParams.AddIgnoredActor( GetOwner() );

	const bool bHasOverlaps = GetWorld()->OverlapMultiByChannel(
	    overlaps, GetActorLocation(), FQuat::Identity, ECC_Entity, FCollisionShape::MakeSphere( AoERadius ), queryParams
	);

	if ( !bHasOverlaps )
	{
		return;
	}

	IEntity* ownerEntity = Cast<IEntity>( GetInstigator() );
	if ( !ownerEntity )
	{
		return;
	}

	TSet<AActor*> damagedActors;

	for ( const FOverlapResult& overlap : overlaps )
	{
		AActor* overlapActor = overlap.GetActor();
		if ( !overlapActor )
		{
			continue;
		}

		if ( damagedActors.Contains( overlapActor ) )
		{
			continue;
		}

		IEntity* enemy = Cast<IEntity>( overlapActor );
		if ( !enemy )
		{
			continue;
		}

		if ( enemy->Stats().IsAlive() && enemy->Team() != ownerEntity->Team() )
		{
			enemy->TakeDamage( Damage_ );
			damagedActors.Add( overlapActor );
		}
	}
}
