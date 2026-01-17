// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Attack/AttackMeleeComponent.h"

#include "Entity.h"
#include "Utilities/TraceChannelMappings.h"

UAttackMeleeComponent::UAttackMeleeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAttackMeleeComponent::BeginPlay()
{
	Super::BeginPlay();

	ActivateSight();
}

void UAttackMeleeComponent::Look()
{
	if ( !OwnerIsValid() )
	{
		return;
	}

	const FVector start = GetOwner()->GetActorLocation();
	const FVector end = start + GetOwner()->GetActorForwardVector() * OwnerEntity_->Stats().AttackRange();

	FHitResult hit;
	FCollisionQueryParams params;
	params.AddIgnoredActor( GetOwner() );

	if ( GetWorld()->LineTraceSingleByChannel( hit, start, end, ECC_Entity, params ) )
	{
		AActor* hitActor = hit.GetActor();
		if ( auto hitEntity = Cast<IEntity>( hitActor ) )
		{
			if ( OwnerEntity_->Team() != hitEntity->Team() )
			{
				EnemyInSight_ = hitActor;
				return;
			}
			return;
		}
	}

	EnemyInSight_ = nullptr;
}

void UAttackMeleeComponent::Attack( TObjectPtr<AActor> hitActor )
{
	if ( !OwnerIsValid() )
	{
		return;
	}

	if ( OwnerEntity_->Stats().OnCooldown() )
	{
		return;
	}

	const auto attackedEntity = Cast<IEntity>( hitActor );
	if ( !attackedEntity )
	{
		return;
	}

	if ( OwnerEntity_->Stats().Team() == attackedEntity->Team() )
	{
		return;
	}

	attackedEntity->TakeDamage( OwnerEntity_->Stats().AttackDamage() );
	OwnerEntity_->Stats().StartCooldown();
}

TObjectPtr<AActor> UAttackMeleeComponent::EnemyInSight() const
{
	return EnemyInSight_;
}

void UAttackMeleeComponent::ActivateSight()
{
	GetWorld()->GetTimerManager().SetTimer(
	    SightTimerHandle_, this, &UAttackMeleeComponent::Look, LookForwardTimeInterval_, true
	);
}

void UAttackMeleeComponent::DeactivateSight()
{
	GetWorld()->GetTimerManager().ClearTimer( SightTimerHandle_ );
	EnemyInSight_ = nullptr;
}
