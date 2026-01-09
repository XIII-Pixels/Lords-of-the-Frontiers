// Fill out your copyright notice in the Description page of Project Settings.

#include "Units/Attack/AttackMeleeComponent.h"

#include "Units/Unit.h"
#include "Utilities/TraceChannelMappings.h"

UAttackMeleeComponent::UAttackMeleeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAttackMeleeComponent::BeginPlay()
{
	Super::BeginPlay();

	Unit_ = Cast<AUnit>( GetOwner() );

	ActivateSight();
}

void UAttackMeleeComponent::Look()
{
	if ( !Unit_ )
	{
		return;
	}

	FVector start = Unit_->GetActorLocation();
	FVector end = start + Unit_->GetActorForwardVector() * Unit_->Stats().AttackRange();

	FHitResult hit;
	FCollisionQueryParams params;
	params.AddIgnoredActor( Unit_ );

	if ( GetWorld()->LineTraceSingleByChannel( hit, start, end, ECC_Entity, params ) )
	{
		AActor* hitActor = hit.GetActor();
		if ( auto hitEntity = Cast<IAttackable>( hitActor ) )
		{
			if ( Unit_->Team() != hitEntity->Team() )
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
	// Probably should use some attack manager, because it would be easier to
	// fetch attack info

	if ( !Unit_ )
	{
		return;
	}

	if ( Unit_->Stats().OnCooldown() )
	{
		return;
	}

	auto attacked = Cast<IAttackable>( hitActor );
	if ( !attacked )
	{
		return;
	}

	if ( Unit_->Stats().Team() == attacked->Team() )
	{
		return;
	}

	attacked->TakeDamage( Unit_->Stats().AttackDamage() );
	Unit_->Stats().StartCooldown();
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
