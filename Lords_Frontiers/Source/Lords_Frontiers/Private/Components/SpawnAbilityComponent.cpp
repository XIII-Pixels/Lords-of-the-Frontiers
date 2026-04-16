// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/SpawnAbilityComponent.h"

#include "Core/CoreManager.h"
#include "Units/Unit.h"
#include "Units/UnitBuilder.h"
#include "Waves/WaveManager.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

void USpawnAbilityComponent::BeginPlay()
{
	Super::BeginPlay();

	const float stopTime = StopTimeBeforeSpawn_ + UnitSpawnInterval_ * SpawnedCount_;
	if ( stopTime > GroupSpawnInterval_ )
	{
		UE_LOG(
		    LogTemp, Error,
		    TEXT( "USpawnAbilityComponent::BeginPlay: invalid SpawnAbilityComponent settings. New group can only be "
		          "spawned after previous group finished spawning" )
		);
		return;
	}

	UnitBuilder_ = NewObject<UUnitBuilder>( this );
	if ( !IsValid( UnitBuilder_ ) )
	{
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(
	    GroupSpawnTimer_, this, &USpawnAbilityComponent::GroupSpawnTick, GroupSpawnInterval_, true, 0
	);
}

void USpawnAbilityComponent::GroupSpawnTick()
{
	const float stopTime = StopTimeBeforeSpawn_ + UnitSpawnInterval_ * SpawnedCount_;
	if ( StopWhileSpawning_ && stopTime > 0 )
	{
		// Stop owner temporarily
		StopUnitMovement();
		GetWorld()->GetTimerManager().SetTimer(
		    MovementTimer_, this, &USpawnAbilityComponent::ResumeUnitMovement,
		    StopTimeBeforeSpawn_ + UnitSpawnInterval_ * SpawnedCount_, false
		);
	}

	if ( UnitSpawnInterval_ > 0 )
	{
		// Start spawning at an interval
		GetWorld()->GetTimerManager().SetTimer(
		    UnitSpawnTimer_, this, &USpawnAbilityComponent::UnitSpawnTick, UnitSpawnInterval_, true,
		    StopTimeBeforeSpawn_
		);
	}
	else
	{
		auto spawnAll = [this]()
		{
			for ( int i = 0; i < SpawnedCount_; ++i )
			{
				ProcessSpawn();
			}
		};

		if ( StopTimeBeforeSpawn_ > 0 )
		{
			// Spawn delayed
			GetWorld()->GetTimerManager().SetTimer(
			    UnitSpawnTimer_, FTimerDelegate::CreateLambda( spawnAll ), StopTimeBeforeSpawn_, false
			);
		}
		else
		{
			// Spawn immediately
			spawnAll();
		}
	}
}

void USpawnAbilityComponent::UnitSpawnTick()
{
	static int remaining = SpawnedCount_;

	SpawnUnit();
	remaining--;

	if ( remaining <= 0 )
	{
		GetWorld()->GetTimerManager().ClearTimer( UnitSpawnTimer_ );
		remaining = SpawnedCount_;
	}
}

void USpawnAbilityComponent::ProcessSpawn() const
{
	if ( SpawnOnlyWhenSeesEnemy_ )
	{
		const AUnit* unit = GetOwner<AUnit>();
		if ( unit && unit->AttackTarget().IsValid() )
		{
			SpawnUnit();
		}
	}
	else
	{
		SpawnUnit();
	}
}

void USpawnAbilityComponent::SpawnUnit() const
{
	if ( !GetWorld() )
	{
		UE_LOG( LogTemp, Error, TEXT( "USpawnAbilityComponent::SpawnUnit: world not found" ) );
		return;
	}

	UnitBuilder_->CreateNewUnit( SpawnedClass_, FindValidTransform() );

	if ( const auto* coreManager = UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<UCoreManager>() )
	{
		if ( const auto* waveManager = coreManager->GetWaveManager() )
		{
			UnitBuilder_->ApplyBuff( waveManager->EnemyBuffs.Find( SpawnedClass_ ) );
		}
	}
	UnitBuilder_->SpawnUnitAndFinish();
}

void USpawnAbilityComponent::StopUnitMovement() const
{
	const AUnit* unit = GetOwner<AUnit>();
	if ( IsValid( unit ) )
	{
		unit->DisableMovement();
	}
}

void USpawnAbilityComponent::ResumeUnitMovement() const
{
	const AUnit* unit = GetOwner<AUnit>();
	if ( IsValid( unit ) )
	{
		unit->EnableMovement();
	}
}

FTransform USpawnAbilityComponent::FindValidTransform() const
{
	if ( !GetOwner() )
	{
		return FTransform();
	}

	float spawnedCapsuleRadius = 34.f;
	float spawnedCapsuleHalfHeight = 88.f;
	if ( SpawnedClass_ )
	{
		if ( const auto* defaultUnit = Cast<AUnit>( SpawnedClass_->GetDefaultObject() ) )
		{
			if ( const UCapsuleComponent* capsule = defaultUnit->FindComponentByClass<UCapsuleComponent>() )
			{
				spawnedCapsuleRadius = capsule->GetUnscaledCapsuleRadius();
				spawnedCapsuleHalfHeight = capsule->GetUnscaledCapsuleHalfHeight();
			}
		}
	}

	float ownerCapsuleRadius = 34.f;
	if ( const auto* unit = GetOwner<AUnit>() )
	{
		if ( const UCapsuleComponent* capsule = unit->FindComponentByClass<UCapsuleComponent>() )
		{
			ownerCapsuleRadius = capsule->GetUnscaledCapsuleRadius();
		}
	}

	FTransform transform = GetOwner()->GetTransform();
	const FVector randomDirection = FMath::VRandCone( GetOwner()->GetActorForwardVector(), PI * 0.5f );
	transform.AddToTranslation( randomDirection * ( spawnedCapsuleRadius + ownerCapsuleRadius * 1.5f ) );

	return UnitBuilder_->FindNonOverlappingSpawnTransform(
	    transform, spawnedCapsuleRadius, spawnedCapsuleHalfHeight, 200.f, 24, false
	);
}
