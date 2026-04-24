// Fill out your copyright notice in the Description page of Project Settings.

#include "Lords_Frontiers/Public/Units/Unit.h"

#include "AI/EntityAIController.h"
#include "AI/UnitAIManager.h"
#include "Core/CoreManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Transform/TransformableHandleUtils.h"
#include "Utilities/TraceChannelMappings.h"
#include "VFX/EntityVFXConfig.h"
#include "Waves/EnemyBuff.h"

#include "Components/CapsuleComponent.h"
#include "Components/FollowComponent.h"
#include "Kismet/GameplayStatics.h"

AUnit::AUnit()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent_ = CreateDefaultSubobject<UCapsuleComponent>( TEXT( "CapsuleCollision" ) );
	CollisionComponent_->SetCollisionObjectType( ECC_Entity );
	SetRootComponent( CollisionComponent_ );

	SkeletalMeshComponent_ = CreateDefaultSubobject<USkeletalMeshComponent>( TEXT( "SkeletalMesh" ) );
	SkeletalMeshComponent_->SetupAttachment( RootComponent );

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	UnitAIControllerClass_ = AEntityAIController::StaticClass();
}

void AUnit::OnConstruction( const FTransform& transform )
{
	Super::OnConstruction( transform );
	AIControllerClass = UnitAIControllerClass_;
}

void AUnit::BeginPlay()
{
	Super::BeginPlay();

	Stats_.SetHealth( Stats_.MaxHealth() );

	FollowComponent_ = FindComponentByClass<UFollowComponent>();
	if ( FollowComponent_ )
	{
		FollowComponent_->SetMaxSpeed( Stats_.MaxSpeed() );
		FollowComponent_->UpdatedComponent = CollisionComponent_;
	}

	TArray<UAttackComponent*> attackComponents;
	GetComponents( attackComponents );

	if ( attackComponents.Num() == 1 )
	{
		AttackComponent_ = attackComponents[0];
	}
	else
	{
		UE_LOG(
		    LogTemp, Error, TEXT( "Unit: number of unit attack components is not equal to 1 (number: %d)" ),
		    attackComponents.Num()
		);
	}

	if ( const UCoreManager* core = UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<UCoreManager>() )
	{
		UnitAIManager_ = core->GetUnitAIManager();
	}

	ResolveVFXDefaults();
}

void AUnit::StartFollowing() const
{
	if ( FollowComponent_ )
	{
		FollowComponent_->StartFollowing();
	}
}

void AUnit::StopFollowing() const
{
	if ( FollowComponent_ )
	{
		FollowComponent_->StopFollowing();
	}
}

void AUnit::Attack( TObjectPtr<AActor> hitActor )
{
	if ( AttackComponent_ && AttackTarget_.IsValid() && !GetWorldTimerManager().IsTimerActive( AttackTimerHandle_ ) )
	{
		if ( AttackPreHitDelay_ > 0.0f && !AttackComponent_->DidSeeTargetLastTick() &&
		     Stats_.CooldownRemaining( GetWorld()->GetTimeSeconds() ) <= AttackPreHitDelay_ )
		{
			GetWorldTimerManager().SetTimer(
			    AttackTimerHandle_, [this, &hitActor]() { Attack( hitActor ); }, AttackPreHitDelay_, false
			);
		}
		else if ( !Stats_.OnCooldown( GetWorld()->GetTimeSeconds() ) )
		{
			AttackComponent_->Attack( hitActor );
		}
	}
}

void AUnit::Animate( float deltaTime ) const
{
	// Only animate attack if unit can attack
	if ( !AttackComponent_ )
	{
		return;
	}

	if ( !SkeletalMeshComponent_->IsPlaying() && AttackTarget_.IsValid() &&
	     Stats_.CooldownRemaining( GetWorld()->GetTimeSeconds() ) <= AttackPreHitDelay_ )
	{
		PlayAnimation( AttackAnimation_ );
	}
}

void AUnit::PlayAnimation( const FAnimationConfig& animation ) const
{
	if ( SkeletalMeshComponent_ && animation.Animation )
	{
		SkeletalMeshComponent_->SetAnimationMode( EAnimationMode::AnimationSingleNode );
		SkeletalMeshComponent_->SetAnimation( animation.Animation );
		SkeletalMeshComponent_->SetPlayRate( animation.PlayRate );
		SkeletalMeshComponent_->Play( false );
	}
}

void AUnit::TakeDamage( int damage )
{
	if ( !Stats_.IsAlive() )
	{
		return;
	}

	Stats_.ApplyDamage( damage );
	if ( !Stats_.IsAlive() )
	{
		OnDeath();
	}
}

void AUnit::OnDeath()
{
	if ( AttackComponent_ )
	{
		AttackComponent_->DeactivateSight();
	}

	if ( FollowComponent_ )
	{
		FollowComponent_->Deactivate();
	}

	if ( CollisionComponent_ )
	{
		CollisionComponent_->SetCollisionEnabled( ECollisionEnabled::NoCollision );
	}

	if ( SkeletalMeshComponent_ )
	{
		SkeletalMeshComponent_->SetVisibility( false, true );
	}

	if ( ResolvedDeathVFXDelay_ > 0.0f )
	{
		GetWorldTimerManager().SetTimer(
		    DeathTimerHandle_,
		    [this]()
		    {
			    SpawnDeathVFX();
			    Destroy();
		    },
		    ResolvedDeathVFXDelay_, false
		);
	}
	else
	{
		SpawnDeathVFX();
		Destroy();
	}
}

void AUnit::SpawnDeathVFX()
{
	if ( !ResolvedDeathVFX_ )
	{
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
	    GetWorld(), ResolvedDeathVFX_, GetActorLocation(), GetActorRotation()
	);
}

UNiagaraSystem* AUnit::GetHitVFX() const
{
	return ResolvedHitVFX_;
}

void AUnit::Tick( float deltaSeconds )
{
	Super::Tick( deltaSeconds );
	Animate( deltaSeconds );
}

void AUnit::ResolveVFXDefaults()
{
	ResolvedDeathVFX_ = DeathVFX_;
	ResolvedHitVFX_ = HitVFX_;

	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( const UEntityVFXConfig* config = core->GetEntityVFXConfig() )
		{
			if ( const FUnitVFXOverride* override = config->UnitOverrides.Find( GetClass() ) )
			{
				if ( !ResolvedDeathVFX_ && override->DeathVFX )
				{
					ResolvedDeathVFX_ = override->DeathVFX;
				}

				if ( !ResolvedHitVFX_ && override->HitVFX )
				{
					ResolvedHitVFX_ = override->HitVFX;
				}

				if ( override->DeathDestroyDelay >= 0.0f )
				{
					ResolvedDeathVFXDelay_ = override->DeathDestroyDelay;
				}
			}

			if ( !ResolvedDeathVFX_ )
			{
				ResolvedDeathVFX_ = config->DefaultUnitDeathVFX;
			}

			if ( !ResolvedHitVFX_ )
			{
				ResolvedHitVFX_ = config->DefaultUnitHitVFX;
			}

			if ( ResolvedDeathVFXDelay_ <= 0.0f )
			{
				ResolvedDeathVFXDelay_ = config->DefaultDeathDestroyDelay;
			}
		}
	}
}

void AUnit::ChangeStats( const FEnemyBuff* buff )
{
	if ( !buff )
	{
		return;
	}

	Stats_.SetMaxHealth(
	    FMath::FloorToInt( Stats_.MaxHealth() * FMath::Pow( buff->HealthMultiplier, buff->SpawnCount ) )
	);

	Stats_.SetAttackRange( Stats_.AttackRange() * FMath::Pow( buff->AttackRangeMultiplier, buff->SpawnCount ) );

	Stats_.SetAttackDamage(
	    FMath::FloorToInt( Stats_.AttackDamage() * FMath::Pow( buff->AttackDamageMultiplier, buff->SpawnCount ) )
	);

	Stats_.SetAttackCooldown(
	    Stats_.AttackCooldown() * FMath::Pow( buff->AttackCooldownMultiplier, buff->SpawnCount )
	);

	Stats_.SetMaxSpeed( Stats_.MaxSpeed() * FMath::Pow( buff->MaxSpeedMultiplier, buff->SpawnCount ) );

	Stats_.Heal( Stats_.MaxHealth() );
}