// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectiles/BaseProjectile.h"

#include "Core/Selection/SelectionManagerComponent.h"
#include "Core/Subsystems/ProjectilePoolSubsystem/ProjectilePoolSubsystem.h"
#include "DrawDebugHelpers.h"
#include "Entity.h"
#include "Utilities/TraceChannelMappings.h"

#include "Components/SphereComponent.h"
#include "Engine/OverlapResult.h"

ABaseProjectile::ABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	PrimaryActorTick.bStartWithTickEnabled = false;

	CollisionComp_ = CreateDefaultSubobject<USphereComponent>( TEXT( "CollisionSphere" ) );
	RootComponent = CollisionComp_;

	CollisionComp_->SetCollisionEnabled( ECollisionEnabled::QueryOnly );
	CollisionComp_->SetCollisionObjectType( ECC_InvisibleVolume );
	CollisionComp_->SetCollisionResponseToAllChannels( ECR_Ignore );
	CollisionComp_->SetCollisionResponseToChannel( ECC_Entity, ECR_Overlap );
	CollisionComp_->SetGenerateOverlapEvents( true );

	CollisionComp_->OnComponentBeginOverlap.AddDynamic( this, &ABaseProjectile::OnCollisionStart );
}

void ABaseProjectile::DeactivateToPool()
{
	bIsActive_ = false;
	SetActorEnableCollision( false );

	SetActorHiddenInGame( true );

	SetActorTickEnabled( false );

	GetWorld()->GetTimerManager().ClearAllTimersForObject( this );

	SetActorLocation( FVector( 0.0f, 0.0f, -10000.0f ) );

	Target_ = nullptr;
	SetInstigator( nullptr );
	SetOwner( nullptr );

	FlightProgress_ = 0.0f;
	FlightDuration_ = 0.0f;
}

void ABaseProjectile::ActivateFromPool()
{
	StartLocation_ = GetActorLocation();

	if ( IsValid( Target_ ) )
	{
		TargetLocation_ = Target_->GetActorLocation();
	}
	else
	{
		TargetLocation_ = StartLocation_ + GetActorForwardVector() * 500.0f;
	}

	FlightDuration_ = FMath::Max( FVector::Dist( StartLocation_, TargetLocation_ ) / Speed_, 0.1f );
	FlightProgress_ = 0.0f;
	bIsActive_ = true;

	SetActorHiddenInGame( false );
	SetActorTickEnabled( true );
	SetActorEnableCollision( true );

	GetWorld()->GetTimerManager().SetTimer(
	    LifetimeTimerHandle, this, &ABaseProjectile::OnLifetimeExpired, MaxLifetime, false
	);
}

void ABaseProjectile::InitializeProjectile(
    AActor* inInstigator, AActor* inTarget, float inDamage, float inSpeed, const FVector& spawnOffset,
    float inSplashRadius
)
{
	Target_ = inTarget;
	Damage_ = FMath::Max( inDamage, 0.0f );
	Speed_ = FMath::Max( inSpeed, 0.0f );
	SplashRadius_ = FMath::Max( inSplashRadius, 0.f );

	SetInstigator( inInstigator->GetInstigator() );
	SetOwner( inInstigator );

	const FVector SpawnLocation = inInstigator->GetActorLocation() + spawnOffset;
	const FVector ToTarget = inTarget->GetActorLocation() - SpawnLocation;

	SetActorLocationAndRotation( SpawnLocation, ToTarget.Rotation() );

	ActivateFromPool();
}

void ABaseProjectile::Tick( float deltaTime )
{
	AActor::Tick( deltaTime );
	if ( !bIsActive_ )
	{
		return;
	}

	if ( bTrackTarget_ && IsValid( Target_ ) )
	{
		if ( IEntity* entity = Cast<IEntity>( Target_ ) )
		{
			if ( !entity->Stats().IsAlive() )
			{
				TargetLocation_ = Target_->GetActorLocation();
				Target_ = nullptr;
			}
		}
	}
	if ( bTrackTarget_ && IsValid( Target_ ) )
	{
		TargetLocation_ = Target_->GetActorLocation();
	}

	FlightProgress_ += deltaTime / FlightDuration_;

	if ( FlightProgress_ >= 1.0f )
	{
		SetActorLocation( TargetLocation_ );
		if ( IsValid( Target_ ) )
		{
			DealDamage( Target_ );
		}
		ReturnToPool();
		return;
	}
	const float t = FlightProgress_;
	const FVector previousLocation = GetActorLocation();

	FVector newLocation;
	newLocation.X = FMath::Lerp( StartLocation_.X, TargetLocation_.X, t );
	newLocation.Y = FMath::Lerp( StartLocation_.Y, TargetLocation_.Y, t );
	newLocation.Z = FMath::Lerp( StartLocation_.Z, TargetLocation_.Z, t ) + ArcHeight_ * 4.0f * t * ( 1.0f - t );

	const FVector moveDirection = newLocation - previousLocation;
	const FRotator newRotation = moveDirection.IsNearlyZero() ? GetActorRotation() : moveDirection.Rotation();

	SetActorLocationAndRotation( newLocation, newRotation );
}

void ABaseProjectile::OnCollisionStart(
    UPrimitiveComponent* overlappedComp, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex,
    bool bFromSweep, const FHitResult& sweepResult
)
{
	if ( !bIsActive_ )
	{
		return;
	}

	if ( IsValid( Target_ ) && otherActor == Target_ )
	{
		DealDamage( otherActor );
		ReturnToPool();
	}
	else if ( !IsValid( Target_ ) )
	{
		IEntity* enemy = Cast<IEntity>( otherActor );
		if ( !enemy || !enemy->Stats().IsAlive() )
		{
			return;
		}

		IEntity* ownerEntity = Cast<IEntity>( GetInstigator() );
		if ( ownerEntity && enemy->Team() != ownerEntity->Team() )
		{
			DealDamage( otherActor );
			ReturnToPool();
		}
	}
}

void ABaseProjectile::DealDamage( AActor* hitActor ) const
{
	if ( IEntity* target = Cast<IEntity>( hitActor ) )
	{
		if ( target->Stats().IsAlive() )
		{
			target->TakeDamage( Damage_ );
		}
	}

	if ( SplashRadius_ <= 0.0f )
	{
		return;
	}

#if ENABLE_DRAW_DEBUG
	DrawDebugSphere( GetWorld(), GetActorLocation(), SplashRadius_, 16, FColor::Red, false, 2.0f, 0, 2.0f );
#endif

	TArray<FOverlapResult> overlaps;
	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor( this );
	queryParams.AddIgnoredActor( GetOwner() );

	const bool bHasOverlaps = GetWorld()->OverlapMultiByChannel(
	    overlaps, GetActorLocation(), FQuat::Identity, ECC_Entity, FCollisionShape::MakeSphere( SplashRadius_ ),
	    queryParams
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
	damagedActors.Add( hitActor );

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

		float distSq = FVector::DistSquared( GetActorLocation(), overlapActor->GetActorLocation() );
		if ( distSq > SplashRadius_ * SplashRadius_ )
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

void ABaseProjectile::ReturnToPool()
{
	if ( !bIsActive_ )
	{
		return;
	}

	if ( UProjectilePoolSubsystem* pool = GetWorld()->GetSubsystem<UProjectilePoolSubsystem>() )
	{
		pool->ReturnProjectile( this );
	}
}

void ABaseProjectile::OnLifetimeExpired()
{
	ReturnToPool();
}