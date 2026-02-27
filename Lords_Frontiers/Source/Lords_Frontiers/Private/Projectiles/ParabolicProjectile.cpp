// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectiles/ParabolicProjectile.h"

#include "Entity.h"
#include "Utilities/TraceChannelMappings.h"

#include "Engine/OverlapResult.h"

AParabolicProjectile::AParabolicProjectile()
{
	ProjectileType = EProjectileType::Catapult;
}

void AParabolicProjectile::ActivateFromPool()
{
	if ( IsValid( Target_ ) )
	{
		TargetLocation_ = Target_->GetActorLocation();
	}
	else
	{
		TargetLocation_ = GetActorLocation() + GetActorForwardVector() * 500.0f;
	}
	StartLocation_ = GetActorLocation();

	float distance = FVector::Dist2D( StartLocation_, TargetLocation_ );

	float maxRange = 0.f;
	if ( IEntity* ownerEntity = Cast<IEntity>( GetInstigator() ) )
	{
		maxRange = ownerEntity->Stats().AttackRange();
	}
	float heightRatio = ( maxRange > 0.f ) ? FMath::Clamp( distance / maxRange, 0.f, 1.f ) : 1.f;
	CurrentArcHeight_ = ArcHeight_ * heightRatio;

	FlightDuration_ = FMath::Max( distance / Speed_, 0.1f );
	FlightProgress_ = 0.0f;

	Super::ActivateFromPool();
}
void AParabolicProjectile::DeactivateToPool()
{
	FlightProgress_ = 0.0f;
	FlightDuration_ = 0.0f;
	CurrentArcHeight_ = 0.0f;

	Super::DeactivateToPool();
}

void AParabolicProjectile::Tick( float deltaTime )
{
	AActor::Tick( deltaTime );

	if ( !bIsActive_ )
	{
		return;
	}

	FlightProgress_ += deltaTime / FlightDuration_;

	if ( FlightProgress_ >= 1.0f )
	{
		SetActorLocation( TargetLocation_ );
		DealDamage( nullptr );
		ReturnToPool();
		return;
	}

	const float t = FlightProgress_;

	const FVector PreviousLocation = GetActorLocation();

	FVector NewLocation;
	NewLocation.X = FMath::Lerp( StartLocation_.X, TargetLocation_.X, t );
	NewLocation.Y = FMath::Lerp( StartLocation_.Y, TargetLocation_.Y, t );
	NewLocation.Z = FMath::Lerp( StartLocation_.Z, TargetLocation_.Z, t ) + CurrentArcHeight_ * 4.0f * t * ( 1.0f - t );

	const FVector MoveDirection = NewLocation - PreviousLocation;
	const FRotator NewRotation = MoveDirection.IsNearlyZero() ? GetActorRotation() : MoveDirection.Rotation();

	SetActorLocationAndRotation( NewLocation, NewRotation );
}
