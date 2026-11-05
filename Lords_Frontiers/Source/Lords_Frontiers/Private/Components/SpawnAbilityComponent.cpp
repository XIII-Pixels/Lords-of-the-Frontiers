// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/SpawnAbilityComponent.h"

#include "Core/CoreManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Units/Unit.h"
#include "Units/UnitBuilder.h"
#include "VFX/EntityVFXConfig.h"
#include "Waves/WaveManager.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

float USpawnAbilityComponent::TimeUntilGroupSpawnStart() const
{
	const UWorld* world = GetWorld();
	if ( world && world->GetTimerManager().IsTimerActive( GroupSpawnTimer_ ) )
	{
		return world->GetTimerManager().GetTimerRemaining( GroupSpawnTimer_ );
	}

	return -1.0f;
}

void USpawnAbilityComponent::StopSpawning()
{
	if ( UWorld* world = GetWorld() )
	{
		FTimerManager& timerManager = world->GetTimerManager();
		timerManager.ClearTimer( GroupSpawnTimer_ );
		timerManager.ClearTimer( UnitSpawnTimer_ );
		timerManager.ClearTimer( MovementTimer_ );
	}
	RemainingInGroup_ = 0;
}

void USpawnAbilityComponent::EndPlay( const EEndPlayReason::Type endPlayReason )
{
	StopSpawning();
	Super::EndPlay( endPlayReason );
}

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

	ResolveVFXDefaults();

	GetWorld()->GetTimerManager().SetTimer(
	    GroupSpawnTimer_, this, &USpawnAbilityComponent::GroupSpawnTick, GroupSpawnInterval_, true
	);
}

void USpawnAbilityComponent::GroupSpawnTick()
{
	if ( auto* unit = GetOwner<AUnit>() )
	{
		unit->GetOnAudioEvent().Broadcast( { unit->AudioTags().SpawnAbility, unit->GetActorLocation() } );
	}

	const float stopTime = StopTimeBeforeSpawn_ + UnitSpawnInterval_ * SpawnedCount_;
	if ( StopWhileSpawning_ && stopTime > 0 )
	{
		// Stop owner temporarily
		StopUnitMovementAndAttack();
		GetWorld()->GetTimerManager().SetTimer(
		    MovementTimer_, this, &USpawnAbilityComponent::ResumeUnitMovementAndAttack,
		    StopTimeBeforeSpawn_ + UnitSpawnInterval_ * SpawnedCount_, false
		);
	}

	if ( const AActor* owner = GetOwner() )
	{
		if ( ResolvedSpawnAbilityVFX_ )
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			    GetWorld(), ResolvedSpawnAbilityVFX_, owner->GetActorLocation(), owner->GetActorRotation()
			);
		}
	}

	if ( SpawnedCount_ <= 0 )
	{
		return;
	}

	if ( UnitSpawnInterval_ > 0 )
	{
		// Start spawning at an interval
		RemainingInGroup_ = SpawnedCount_;
		GetWorld()->GetTimerManager().SetTimer(
			UnitSpawnTimer_, this, &USpawnAbilityComponent::UnitSpawnTick, UnitSpawnInterval_, true,
			StopTimeBeforeSpawn_
		);
	}
	else if ( StopTimeBeforeSpawn_ > 0 )
	{
		// Spawn delayed. UObject delegate (not a lambda with raw this): it is skipped
		// automatically if the component is destroyed before the timer fires.
		GetWorld()->GetTimerManager().SetTimer(
			UnitSpawnTimer_, this, &USpawnAbilityComponent::SpawnWholeGroup, StopTimeBeforeSpawn_, false
		);
	}
	else
	{
		// Spawn immediately
		SpawnWholeGroup();
	}
}

void USpawnAbilityComponent::SpawnWholeGroup()
{
	for ( int32 i = 0; i < SpawnedCount_; ++i )
	{
		ProcessSpawn();
	}
}

void USpawnAbilityComponent::UnitSpawnTick()
{
	ProcessSpawn();
	--RemainingInGroup_;

	if ( RemainingInGroup_ <= 0 )
	{
		if ( UWorld* world = GetWorld() )
		{
			world->GetTimerManager().ClearTimer( UnitSpawnTimer_ );
		}
		RemainingInGroup_ = 0;
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

	if ( !IsValid( UnitBuilder_ ) || !SpawnedClass_ )
	{
		return;
	}

	// A dead summoner must not summon anymore
	if ( const AUnit* ownerUnit = GetOwner<AUnit>() )
	{
		if ( ownerUnit->IsActorBeingDestroyed() || !ownerUnit->Stats().IsAlive() )
		{
			return;
		}
	}

	UnitBuilder_->CreateNewUnit( SpawnedClass_, FindValidTransform() );

	AWaveManager* waveManager = nullptr;
	if ( const UCoreManager* coreManager = UCoreManager::Get( this ) )
	{
		waveManager = coreManager->GetWaveManager();
	}

	if ( waveManager )
	{
		UnitBuilder_->ApplyBuff( waveManager->FindBuffForCurrentWave( SpawnedClass_ ) );
	}

	const TWeakObjectPtr<AUnit> spawned = UnitBuilder_->SpawnUnitAndFinish();

	// Summoned units belong to the current wave: they keep it alive and are removed with it (#818, #735)
	if ( waveManager && spawned.IsValid() )
	{
		waveManager->RegisterSpawnedUnit( spawned.Get() );
	}
}

void USpawnAbilityComponent::StopUnitMovementAndAttack() const
{
	AUnit* unit = GetOwner<AUnit>();
	if ( IsValid( unit ) )
	{
		unit->DisableMovement();
		unit->DisableAttack();
	}
}

void USpawnAbilityComponent::ResumeUnitMovementAndAttack() const
{
	AUnit* unit = GetOwner<AUnit>();
	if ( IsValid( unit ) )
	{
		unit->EnableMovement();
		unit->EnableAttack();
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

void USpawnAbilityComponent::ResolveVFXDefaults()
{
	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( const UEntityVFXConfig* config = core->GetEntityVFXConfig() )
		{
			if ( const FUnitVFXOverride* override = config->UnitOverrides.Find( GetClass() ) )
			{
				if ( !ResolvedSpawnAbilityVFX_ && override->SpawnAbilityVFX )
				{
					ResolvedSpawnAbilityVFX_ = override->SpawnAbilityVFX;
				}
			}

			if ( !ResolvedSpawnAbilityVFX_ )
			{
				ResolvedSpawnAbilityVFX_ = config->DefaultUnitSpawnAbilityVFX;
			}
		}
	}
}
