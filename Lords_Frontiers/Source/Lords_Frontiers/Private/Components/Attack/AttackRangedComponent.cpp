// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Attack/AttackRangedComponent.h"

#include "Core/Subsystems/ProjectilePoolSubsystem/ProjectilePoolSubsystem.h"
#include "Entity.h"
#include "Projectiles/BaseProjectile.h"
#include "Utilities/TraceChannelMappings.h"

#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

UAttackRangedComponent::UAttackRangedComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SightSphere_ = CreateDefaultSubobject<USphereComponent>( "Sphere Component" );
	SightSphere_->SetCollisionEnabled( ECollisionEnabled::QueryOnly );
	SightSphere_->SetCollisionObjectType( ECC_InvisibleVolume );
	SightSphere_->SetCollisionResponseToAllChannels( ECR_Ignore );
	SightSphere_->SetCollisionResponseToChannel( ECC_Entity, ECR_Overlap );
	SightSphere_->SetGenerateOverlapEvents( true );
}

void UAttackRangedComponent::OnRegister()
{
	Super::OnRegister();

	if ( GetOwner() )
	{
		SightSphere_->SetupAttachment( GetOwner()->GetRootComponent() );
		if ( OwnerEntity_ )
		{
			SightSphere_->SetSphereRadius( OwnerEntity_->Stats().AttackRange() );
		}
	}
}

void UAttackRangedComponent::BeginPlay()
{
	Super::BeginPlay();

	ActivateSight();
}

void UAttackRangedComponent::Attack( TObjectPtr<AActor> hitActor )
{
	if ( !OwnerIsValid() )
	{
		return;
	}

	if ( OwnerEntity_->Stats().OnCooldown() || bBurstInProgress_ )
	{
		return;
	}

	if ( !EnemyInSight_ || !ProjectileClass_ )
	{
		return;
	}

	const int32 burstCount = OwnerEntity_->Stats().BurstCount();

	if ( burstCount <= 1 )
	{
		FireSingleProjectile( EnemyInSight_ );
		OwnerEntity_->Stats().StartCooldown();
		return;
	}

	BurstTargets_.Empty();

	if ( OwnerEntity_->Stats().BurstTargetMode() == EBurstTargetMode::SameTarget )
	{
		for ( int32 i = 0; i < burstCount; ++i )
		{
			BurstTargets_.Add( EnemyInSight_ );
		}
	}
	else
	{
		TArray<TObjectPtr<AActor>> uniqueTargets = FindNeighborTargets( burstCount );
		if ( uniqueTargets.Num() == 0 )
		{
			return;
		}

		for ( int32 i = 0; i < burstCount; ++i )
		{
			BurstTargets_.Add( uniqueTargets[i % uniqueTargets.Num()] );
		}
	}

	bBurstInProgress_ = true;
	CurrentBurstIndex_ = 0;
	FireNextBurstShot();
}

TObjectPtr<AActor> UAttackRangedComponent::EnemyInSight() const
{
	return EnemyInSight_;
}

void UAttackRangedComponent::ActivateSight()
{
	GetWorld()->GetTimerManager().SetTimer(
	    SightTimerHandle_, this, &UAttackRangedComponent::Look, LookForwardTimeInterval_, true
	);
}

void UAttackRangedComponent::DeactivateSight()
{
	GetWorld()->GetTimerManager().ClearTimer( SightTimerHandle_ );
	GetWorld()->GetTimerManager().ClearTimer( BurstTimerHandle_ );
	bBurstInProgress_ = false;
	BurstTargets_.Empty();
	EnemyInSight_ = nullptr;
}

void UAttackRangedComponent::Look()
{
	if ( !OwnerIsValid() )
	{
		return;
	}

	EnemyInSight_ = nullptr;

	TArray<AActor*> overlappingActors;
	SightSphere_->GetOverlappingActors( overlappingActors, AActor::StaticClass() );

	float minDistance = -1.0f;
	for ( auto actor : overlappingActors )
	{
		if ( actor == GetOwner() )
		{
			continue;
		}

		if ( CanSeeEnemy( actor ) )
		{
			float distance = FVector::Distance( GetOwner()->GetActorLocation(), actor->GetActorLocation() );
			if ( !EnemyInSight_ || distance < minDistance )
			{
				EnemyInSight_ = actor;
				minDistance = distance;
			}
		}
	}

	// UE_LOG( LogTemp, Log, TEXT( "Enemy in sight: %s" ), *GetNameSafe( EnemyInSight_ ) );
}

bool UAttackRangedComponent::CanSeeEnemy( TObjectPtr<AActor> actor ) const
{
	// it is assumed the actor is inside the sight sphere
	if ( !OwnerIsValid() )
	{
		return false;
	}

	const auto enemy = Cast<IEntity>( actor );
	if ( enemy && enemy->Stats().IsAlive() && enemy->Team() != OwnerEntity_->Team() )
	{
		if ( !bCanAttackBackward_ ) // look in half-circle in front of unit
		{
			const float dot = FVector::DotProduct(
			    actor->GetActorLocation() - GetOwner()->GetActorLocation(), GetOwner()->GetActorForwardVector()
			);
			if ( dot > 0 )
			{
				return true;
			}
		}
		else // look around unit
		{
			return true;
		}
	}
	return false;
}

void UAttackRangedComponent::FireSingleProjectile( TObjectPtr<AActor> target )
{
	UWorld* world = GetOwner()->GetWorld();
	if ( !world || !target || !ProjectileClass_ )
	{
		return;
	}

	UProjectilePoolSubsystem* pool = world->GetSubsystem<UProjectilePoolSubsystem>();
	if ( !pool )
	{
		return;
	}

	ABaseProjectile* projectile = pool->AcquireProjectile( ProjectileClass_ );
	if ( !projectile )
	{
		return;
	}

	projectile->InitializeProjectile(
	    GetOwner(), target, OwnerEntity_->Stats().AttackDamage(), ProjectileSpeed_, ProjectileSpawnPosition_,
	    OwnerEntity_->Stats().SplashRadius()
	);
}

TArray<TObjectPtr<AActor>> UAttackRangedComponent::FindNeighborTargets( int32 count ) const
{
	TArray<TObjectPtr<AActor>> result;
	TArray<AActor*> overlappingActors;
	SightSphere_->GetOverlappingActors( overlappingActors, APawn::StaticClass() );

	struct FActorDistance
	{
		TObjectPtr<AActor> Actor;
		float Distance;
	};
	TArray<FActorDistance> candidates;

	const FVector ownerLocation = GetOwner()->GetActorLocation();
	for ( auto actor : overlappingActors )
	{
		if ( actor == GetOwner() )
		{
			continue;
		}
		if ( CanSeeEnemy( actor ) )
		{
			float distance = FVector::DistSquared( ownerLocation, actor->GetActorLocation() );
			candidates.Add( { actor, distance } );
		}
	}
	const int32 resultCount = FMath::Min( count, candidates.Num() );
	std::partial_sort(
	    candidates.GetData(), candidates.GetData() + resultCount, candidates.GetData() + candidates.Num(),
	    []( const FActorDistance& a, const FActorDistance& b ) { return a.Distance < b.Distance; }
	);

	for ( int32 i = 0; i < resultCount; ++i )
	{
		result.Add( candidates[i].Actor );
	}
	
	return result;
}

void UAttackRangedComponent::FireNextBurstShot()
{
	if ( !OwnerIsValid() || !bBurstInProgress_ )
	{
		bBurstInProgress_ = false;
		BurstTargets_.Empty();
		return;
	}

	if (CurrentBurstIndex_ >= BurstTargets_.Num())
	{
		OwnerEntity_->Stats().StartCooldown();
		bBurstInProgress_ = false;
		BurstTargets_.Empty();
		return;
	}

	TObjectPtr<AActor> target = BurstTargets_[CurrentBurstIndex_];

	if (!IsValid(target) || !Cast<IEntity>(target) || !Cast<IEntity>(target)->Stats().IsAlive())
	{
		target = EnemyInSight_;
	}

	if (IsValid(target))
	{
		FireSingleProjectile( target );
	}
	++CurrentBurstIndex_;
	if (CurrentBurstIndex_ >= BurstTargets_.Num())
	{
		OwnerEntity_->Stats().StartCooldown();
		bBurstInProgress_ = false;
		BurstTargets_.Empty();
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(
		    BurstTimerHandle_, this, &UAttackRangedComponent::FireNextBurstShot, OwnerEntity_->Stats().BurstDelay(),
		    false
		);
	}
}