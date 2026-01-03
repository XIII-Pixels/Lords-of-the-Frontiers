// Fill out your copyright notice in the Description page of Project Settings.

#include "Units/Attack/AttackMeleeComponent.h"

UAttackMeleeComponent::UAttackMeleeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAttackMeleeComponent::BeginPlay()
{
	Super::BeginPlay();

	Unit_ = Cast<AUnit>(GetOwner());

	GetWorld()->GetTimerManager().SetTimer(
		SightTimerHandle_,
		this,
		&UAttackMeleeComponent::SightTick,
		LookForwardTimeInterval_,
		true );
}

void UAttackMeleeComponent::SightTick()
{
	LookForward();

	GetWorld()->GetTimerManager().SetTimer(
		SightTimerHandle_,
		this,
		&UAttackMeleeComponent::SightTick,
		LookForwardTimeInterval_,
		true );
}

void UAttackMeleeComponent::LookForward()
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

	if ( GetWorld()->LineTraceSingleByChannel( hit, start, end, ECC_Pawn, params ) )
	{
		AActor* hitActor = hit.GetActor();
		if ( auto hitEntity = Cast<IAttackable>( hitActor ) )
		{
			if ( Unit_->Team() != hitEntity->Team() )
			{
				EnemyInSight_ = hitActor;
				// UE_LOG( LogTemp, Display, TEXT( "Enemy seen" ) );
				return;
			}
			// UE_LOG( LogTemp, Display, TEXT( "Ally seen" ) );
			return;
		}
		// UE_LOG( LogTemp, Display, TEXT( "Actor seen" ) );
	}

	EnemyInSight_ = nullptr;
}

void UAttackMeleeComponent::Attack(TObjectPtr<AActor> hitActor)
{
	// Probably should use some attack manager, because it would be easier to fetch attack info
	
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

	// UE_LOG( LogTemp, Display, TEXT( "Damage applied" ) );

	attacked->TakeDamage( Unit_->Stats().AttackDamage() );
	Unit_->Stats().StartCooldown();
}

TObjectPtr<AActor> UAttackMeleeComponent::EnemyInSight() const
{
	return EnemyInSight_;
}
