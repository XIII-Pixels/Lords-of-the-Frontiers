// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectiles/Projectile.h"

#include "Attackable.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnitConversion.h"
#include "Utilities/TraceChannelMappings.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent_ = CreateDefaultSubobject<USphereComponent>( TEXT( "Collision Sphere" ) );
	RootComponent = CollisionComponent_;
	CollisionComponent_->SetCollisionEnabled( ECollisionEnabled::QueryOnly );
	CollisionComponent_->SetCollisionObjectType( ECC_InvisibleVolume );
	CollisionComponent_->SetCollisionResponseToAllChannels( ECR_Ignore );
	CollisionComponent_->SetCollisionResponseToChannel( ECC_Entity, ECR_Overlap );
	CollisionComponent_->SetGenerateOverlapEvents( true );

	CollisionComponent_->OnComponentBeginOverlap.AddDynamic( this, &AProjectile::OnCollisionStart );
}

void AProjectile::Initialize( TObjectPtr<AActor> target, float damage, float speed, bool splash )
{
	Target_ = target;
	Damage_ = FMath::Max( damage, 0.0f );
	Speed_ = FMath::Max( speed, 0.0f );
	bSplash_ = splash;
}

void AProjectile::Launch()
{
	bIsFollowing_ = true;
}

void AProjectile::Tick( float deltaTime )
{
	Super::Tick( deltaTime );

	if ( !bIsFollowing_ )
	{
		return;
	}

	if ( !Target_ )
	{
		Destroy();
		return;
	}

	FVector toTarget = Target_->GetActorLocation() - GetActorLocation();
	toTarget.Normalize();

	const FVector deltaMove = toTarget * Speed_ * deltaTime;
	AddActorWorldOffset( deltaMove, true );

	const FRotator facingRotator = toTarget.Rotation();
	SetActorRotation( facingRotator, ETeleportType::None );
}

void AProjectile::DealDamage( TObjectPtr<AActor> target ) const
{
	if ( auto enemy = Cast<IAttackable>( target ) )
	{
		enemy->TakeDamage( Damage_ );
	}
}

void AProjectile::OnCollisionStart(
    UPrimitiveComponent* overlappedComp, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex,
    bool bFromSweep, const FHitResult& sweepResult
)
{
	if ( !bIsFollowing_ )
	{
		return;
	}

	GEngine->AddOnScreenDebugMessage( -1, 5.0f, FColor::Blue, "Projectile collision start" );
	if ( otherActor == Target_ )
	{
		DealDamage( otherActor );
		Destroy();
	}
}
