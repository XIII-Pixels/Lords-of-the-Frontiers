// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Attack/AttackRangedComponent.h"

#include "Entity.h"
#include "Projectiles/Projectile.h"
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

	if ( OwnerEntity_->Stats().OnCooldown() )
	{
		return;
	}

	UWorld* world = GetOwner()->GetWorld();
	if ( !world )
	{
		return;
	}

	FTransform spawnTransform = GetOwner()->GetTransform();
	spawnTransform.AddToTranslation( ProjectileSpawnPosition_ );

	if ( EnemyInSight_ && ProjectileClass_ )
	{
		const FActorSpawnParameters spawnParams;
		const auto projectile = world->SpawnActor<AProjectile>( ProjectileClass_, spawnTransform, spawnParams );
		projectile->Initialize( EnemyInSight(), OwnerEntity_->Stats().AttackDamage(), ProjectileSpeed_ );
		projectile->Launch();
		OwnerEntity_->Stats().StartCooldown();
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

	UE_LOG( LogTemp, Log, TEXT( "Enemy in sight: %s" ), *GetNameSafe( EnemyInSight_ ) );
}

bool UAttackRangedComponent::CanSeeEnemy( TObjectPtr<AActor> actor ) const
{
	// it is assumed the actor is inside the sight sphere
	if ( !OwnerIsValid() )
	{
		return false;
	}

	auto enemy = Cast<IEntity>( actor );
	if ( enemy && enemy->Team() != OwnerEntity_->Team() )
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
