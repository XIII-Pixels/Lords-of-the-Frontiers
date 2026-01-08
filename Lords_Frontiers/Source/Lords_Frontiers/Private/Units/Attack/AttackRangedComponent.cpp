// Fill out your copyright notice in the Description page of Project Settings.

#include "Units/Attack/AttackRangedComponent.h"

#include "Attackable.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Projectiles/Projectile.h"
#include "Units/Unit.h"
#include "Utilities/TraceChannelMappings.h"

UAttackRangedComponent::UAttackRangedComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

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

	Unit_ = Cast<AUnit>( GetOwner() );

	if ( Unit_ )
	{
		SightSphere_->SetupAttachment( Unit_->GetRootComponent() );
		SightSphere_->SetSphereRadius( Unit_->Stats().AttackRange() );
	}
}

void UAttackRangedComponent::BeginPlay()
{
	Super::BeginPlay();

	ActivateSight();
}

void UAttackRangedComponent::Attack( TObjectPtr<AActor> hitActor )
{
	if ( !Unit_ )
	{
		return;
	}

	if ( Unit_->Stats().OnCooldown() )
	{
		return;
	}

	UWorld* world = Unit_->GetWorld();
	if ( !world )
	{
		return;
	}

	FTransform spawnTransform = Unit_->GetTransform();
	spawnTransform.AddToTranslation( FVector( 0.0f, 0.0f, Unit_->GetDefaultHalfHeight() ) );

	if ( EnemyInSight_ && ProjectileClass_ )
	{
		const FActorSpawnParameters spawnParams;
		const auto projectile = world->SpawnActor<AProjectile>( ProjectileClass_, spawnTransform, spawnParams );
		projectile->Initialize( EnemyInSight(), Unit_->Stats().AttackDamage(), ProjectileSpeed_ );
		projectile->Launch();
		Unit_->Stats().StartCooldown();
	}
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
	EnemyInSight_ = nullptr;
}

void UAttackRangedComponent::Look()
{
	if ( !Unit_ )
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
			float distance = FVector::Distance( Unit_->GetActorLocation(), actor->GetActorLocation() );
			if ( !EnemyInSight_ || distance < minDistance )
			{
				EnemyInSight_ = actor;
				minDistance = distance;
			}
		}
	}

	UE_LOG( LogTemp, Log, TEXT( "Enemy in sight: %s" ), *GetNameSafe( EnemyInSight_ ) );
}

bool UAttackRangedComponent::CanSeeEnemy( TObjectPtr<AActor> actor ) const
{
	// it is assumed the actor is inside the sight sphere

	auto enemy = Cast<IAttackable>( actor );
	if ( enemy && enemy->Team() != Unit_->Team() )
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
