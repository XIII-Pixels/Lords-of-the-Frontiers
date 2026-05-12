// Fill out your copyright notice in the Description page of Project Settings.

#include "Lords_Frontiers/Public/Units/Unit.h"

#include "AI/EntityAIController.h"
#include "AI/Path/PathPointsManager.h"
#include "AI/UnitAIManager.h"
#include "Core/CoreManager.h"
#include "Core/Subsystems/HealthBarPoolSubsystem/HealthBarPoolSubsystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Transform/TransformableHandleUtils.h"
#include "UI/HealthBar/HealthBarConfigDataAsset.h"
#include "Utilities/TraceChannelMappings.h"
#include "VFX/EntityVFXConfig.h"
#include "Waves/EnemyBuff.h"

#include "Components/Attack/UnitAttackRangedComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/FollowComponent.h"
#include "Components/SpawnAbilityComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/AudioTags.h"
#include "sound/SoundEffectManager.h"

AUnit::AUnit()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent_ = CreateDefaultSubobject<UCapsuleComponent>( TEXT( "CapsuleCollision" ) );
	CollisionComponent_->SetCollisionObjectType( ECC_Entity );
	SetRootComponent( CollisionComponent_ );

	AttackComponent_ = CreateDefaultSubobject<UUnitAttackRangedComponent>( TEXT( "Attack Ranged " ) );
	AttackComponent_->SetupAttachment( RootComponent );

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

	SpawnAbilityComponent_ = FindComponentByClass<USpawnAbilityComponent>();

	if ( const UCoreManager* core = UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<UCoreManager>() )
	{
		UnitAIManager_ = core->GetUnitAIManager();
	}

	ResolveVFXDefaults();

	const UWorld* world = GetWorld();
	if ( !world )
	{
		UE_LOG( LogTemp, Error, TEXT( "AUnit::BeginPlay: world not found" ) );
	}

	if ( HealthBarConfig_ )
	{
		HealthBarSubscription_ = Stats_.OnHealthChanged.AddWeakLambda(
		    this,
		    [this, world]( int /*newHealth*/, int /*maxHealth*/ )
		    {
			    if ( world )
			    {
				    if ( UHealthBarPoolSubsystem* pool = world->GetSubsystem<UHealthBarPoolSubsystem>() )
				    {
					    pool->ShowFor( this, HealthBarConfig_ );
				    }
			    }
		    }
		);

		if ( bIsBoss_ )
		{
			if ( UHealthBarPoolSubsystem* pool = GetWorld()->GetSubsystem<UHealthBarPoolSubsystem>() )
			{
				pool->ShowFor( this, HealthBarConfig_ );
			}
		}
	}

	if ( const UGameInstance* gameInstance = UGameplayStatics::GetGameInstance( world ) )
	{
		if ( USoundEffectManager* sfxManager = gameInstance->GetSubsystem<USoundEffectManager>() )
		{
			sfxManager->RegisterObject( this );
		}
	}

    OnAudioEvent_.Broadcast( { AudioTags_.Spawn, GetActorLocation() } );

    PlayAnimationIdle();

	SpawnSpawnVFX();
}

void AUnit::EndPlay( const EEndPlayReason::Type endPlayReason )
{
	Super::EndPlay( endPlayReason );

	if ( const UGameInstance* gameInstance = UGameplayStatics::GetGameInstance( GetWorld() ) )
	{
		if ( USoundEffectManager* sfxManager = gameInstance->GetSubsystem<USoundEffectManager>() )
		{
			sfxManager->UnregisterObject( this );
		}
	}
}

void AUnit::PostInitProperties()
{
	Super::PostInitProperties();

	ResolveAudioTags();
  
	if ( IsValid( IdleAnimation_.Animation ) )
	{
		bIdleIsAnimated_ = true;
	}
	else
	{
		bIdleIsAnimated_ = false;
		IdleAnimation_ = AttackAnimation_;
	}
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

void AUnit::EnableMovement() const
{
	if ( FollowComponent_ )
	{
		FollowComponent_->Activate();
	}
}

void AUnit::DisableMovement() const
{
	if ( FollowComponent_ )
	{
		FollowComponent_->Deactivate();
	}
}

void AUnit::EnableAttack()
{
	if ( const auto* world = GetWorld() )
	{
		Stats_.StartCooldown( world->GetTimeSeconds() );
	}
	bCanAttack = true;
}

void AUnit::DisableAttack()
{
	bCanAttack = false;
}

void AUnit::Attack( TObjectPtr<AActor> hitActor )
{
	if ( !bCanAttack )
	{
		return;
	}

	if ( AttackComponent_ && AttackTarget_.IsValid() && !GetWorldTimerManager().IsTimerActive( AttackTimerHandle_ ) )
	{
		if ( AttackPreHitDelay_ > 0.0f && !AttackComponent_->DidSeeTargetLastTick() &&
		     Stats_.CooldownRemaining( GetWorld()->GetTimeSeconds() ) <= AttackPreHitDelay_ )
		{
			GetWorldTimerManager().SetTimer(
			    AttackTimerHandle_, [this, hitActor]() { Attack( hitActor ); }, AttackPreHitDelay_, false
			);
		}
		else if ( !Stats_.OnCooldown( GetWorld()->GetTimeSeconds() ) )
		{
			if ( bool success = AttackComponent_->Attack( hitActor ) )
			{
				OnAudioEvent_.Broadcast( { AudioTags_.Attack, GetActorLocation() } );
			}
		}
	}
}

void AUnit::Animate()
{
	if ( bPlayingIdleAnimation )
	{
		bool startedAnimation = false;
		if ( SpawnAbilityComponent_.IsValid() &&
			 SpawnAbilityComponent_->TimeUntilGroupSpawnStart() <= PreSpawnAbilityDelay_ )
		{
			startedAnimation = PlayAnimation( SpawnAbilityAnimation_ );
		}
		else if ( IsValid( AttackComponent_ ) && AttackTarget_.IsValid() && Stats_.CooldownRemaining( GetWorld()->GetTimeSeconds() ) <= AttackPreHitDelay_ )
		{
			startedAnimation = PlayAnimation( AttackAnimation_ );
		}

		if ( startedAnimation )
		{
			bPlayingIdleAnimation = false;
		}
	}

	if ( !SkeletalMeshComponent_->IsPlaying() )
	{
		// Play idle by default
		PlayAnimationIdle();
	}
}

bool AUnit::PlayAnimation( const FAnimationConfig& animation ) const
{
	if ( SkeletalMeshComponent_ && animation.Animation )
	{
		SkeletalMeshComponent_->SetAnimationMode( EAnimationMode::AnimationSingleNode );
		SkeletalMeshComponent_->SetAnimation( animation.Animation );
		SkeletalMeshComponent_->SetPlayRate( animation.PlayRate );
		SkeletalMeshComponent_->Play( false );

		if ( animation.bStopWhileAnimating )
		{
			DisableMovement();
		}
		else
		{
			EnableMovement();
		}
		return true;
	}
	return false;
}

void AUnit::ResolveAudioTags()
{
	if ( !AudioTags_.Selected.IsValid() )
	{
		AudioTags_.Selected = AudioTags::SFX_UNIT_DEFAULT_SELECTED;
	}

	if ( !AudioTags_.Spawn.IsValid() )
	{
		AudioTags_.Spawn = AudioTags::SFX_UNIT_DEFAULT_SPAWN;
	}

	if ( !AudioTags_.Death.IsValid() )
	{
		AudioTags_.Death = AudioTags::SFX_UNIT_DEFAULT_DEATH;
	}

	if ( !AudioTags_.Attack.IsValid() )
	{
		AudioTags_.Attack = AudioTags::SFX_UNIT_DEFAULT_ATTACK;
	}

	if ( !AudioTags_.TakeDamage.IsValid() )
	{
		AudioTags_.TakeDamage = AudioTags::SFX_UNIT_DEFAULT_TAKEDAMAGE;
	}

	if ( !AudioTags_.SpawnAbility.IsValid() )
	{
		AudioTags_.SpawnAbility = AudioTags::SFX_UNIT_DEFAULT_SPAWNABILITY;
	}
}

void AUnit::TakeDamage( int damage, AActor* /*instigator*/ )
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
	else
	{
		OnAudioEvent_.Broadcast( { AudioTags_.TakeDamage, GetActorLocation() } );
	}
}

void AUnit::OnDeath()
{
	Stats_.OnHealthChanged.Remove( HealthBarSubscription_ );
	HealthBarSubscription_.Reset();

	if ( UnitAIManager_.IsValid() )
	{
		UnitAIManager_->PathPointsManager()->ReleasePathPoints( Path(), GetClass() );
	}

	if ( UWorld* world = GetWorld() )
	{
		if ( UHealthBarPoolSubsystem* pool = world->GetSubsystem<UHealthBarPoolSubsystem>() )
		{
			pool->HideFor( this );
		}
	}

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

	OnAudioEvent_.Broadcast( { AudioTags_.Death, GetActorLocation() } );

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

void AUnit::SpawnSpawnVFX()
{
	if ( !ResolvedSpawnVFX_ )
	{
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
	    GetWorld(), ResolvedSpawnVFX_, GetActorLocation(), GetActorRotation()
	);
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

bool AUnit::PlayAnimationIdle()
{
	if ( !IsValid( IdleAnimation_.Animation ) )
	{
		return false;
	}

	SkeletalMeshComponent_->SetAnimationMode( EAnimationMode::AnimationSingleNode );
	SkeletalMeshComponent_->SetAnimation( IdleAnimation_.Animation );

	if ( bIdleIsAnimated_ )
	{
		SkeletalMeshComponent_->SetPlayRate( IdleAnimation_.PlayRate );
		SkeletalMeshComponent_->Play( true );
	}
	else
	{
		SkeletalMeshComponent_->SetPosition( 0 );
	}
	bPlayingIdleAnimation = true;

	if ( IdleAnimation_.bStopWhileAnimating )
	{
		DisableMovement();
	}
	else
	{
		EnableMovement();
	}
	return true;
}

bool AUnit::PlayAnimationAttack()
{
	bPlayingIdleAnimation = false;
	return PlayAnimation( AttackAnimation_ );
}

bool AUnit::PlayAnimationSpawnAbility()
{
	bPlayingIdleAnimation = false;
	return PlayAnimation( SpawnAbilityAnimation_ );
}

void AUnit::Tick( float deltaSeconds )
{
	Super::Tick( deltaSeconds );
	Animate();
}

void AUnit::ResolveVFXDefaults()
{
	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( const UEntityVFXConfig* config = core->GetEntityVFXConfig() )
		{
			if ( const FUnitVFXOverride* override = config->UnitOverrides.Find( GetClass() ) )
			{
				if ( override->SpawnVFX )
				{
					ResolvedSpawnVFX_ = override->SpawnVFX;
				}

				if ( override->DeathVFX )
				{
					ResolvedDeathVFX_ = override->DeathVFX;
				}

				if ( override->HitVFX )
				{
					ResolvedHitVFX_ = override->HitVFX;
				}

				if ( override->DeathDestroyDelay >= 0.0f )
				{
					ResolvedDeathVFXDelay_ = override->DeathDestroyDelay;
				}
			}

			if ( !ResolvedSpawnVFX_ )
			{
				ResolvedSpawnVFX_ = config->DefaultUnitSpawnVFX;
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

	Stats_.SetMaxHealth( FMath::FloorToInt( Stats_.MaxHealth() * buff->HealthMultiplier ) );

	Stats_.SetAttackRange( Stats_.AttackRange() * buff->AttackRangeMultiplier );

	Stats_.SetAttackDamage( FMath::FloorToInt( Stats_.AttackDamage() * buff->AttackDamageMultiplier ) );

	Stats_.SetAttackCooldown( Stats_.AttackCooldown() * buff->AttackCooldownMultiplier );

	Stats_.SetMaxSpeed( Stats_.MaxSpeed() * buff->MaxSpeedMultiplier );

	Stats_.Heal( Stats_.MaxHealth() );
}
