// Fill out your copyright notice in the Description page of Project Settings.

#include "Lords_Frontiers/Public/Units/Unit.h"

#include "AI/EntityAIController.h"
#include "AI/Path/Path.h"
#include "AI/Path/PathPointsManager.h"
#include "AI/Path/PathTargetPoint.h"
#include "AI/UnitAIManager.h"
#include "Core/CoreManager.h"
#include "Core/EntityVFXConfig.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Transform/TransformableHandleUtils.h"
#include "Utilities/TraceChannelMappings.h"
#include "Waves/EnemyBuff.h"

#include "Components/CapsuleComponent.h"
#include "Components/FollowComponent.h"
#include "Kismet/GameplayStatics.h"

AUnit::AUnit()
{
	CollisionComponent_ = CreateDefaultSubobject<UCapsuleComponent>( TEXT( "CapsuleCollision" ) );
	SetRootComponent( CollisionComponent_ );

	CollisionComponent_->SetCollisionObjectType( ECC_Entity );

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

	VisualMesh_ = Cast<USceneComponent>( GetComponentByClass( UMeshComponent::StaticClass() ) );

	ResolveVFXDefaults();
}

void AUnit::Tick( float deltaSeconds )
{
	Super::Tick( deltaSeconds );

	if ( FollowedTarget_.Get() && FollowedTarget_.Get()->IsA( APathTargetPoint::StaticClass() ) && IsCloseToTarget() )
	{
		FollowNextPathTarget();
	}
}

void AUnit::StartFollowing()
{
	if ( FollowComponent_ )
	{
		FollowComponent_->StartFollowing();
	}
}

void AUnit::StopFollowing()
{
	if ( FollowComponent_ )
	{
		FollowComponent_->StopFollowing();
	}
}

void AUnit::Attack( TObjectPtr<AActor> hitActor )
{
	if ( AttackComponent_ )
	{
		AttackComponent_->Attack( hitActor );
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

FEntityStats& AUnit::Stats()
{
	return Stats_;
}

ETeam AUnit::Team()
{
	return Stats_.Team();
}

TObjectPtr<AActor> AUnit::EnemyInSight() const
{
	if ( AttackComponent_ )
	{
		return AttackComponent_->EnemyInSight();
	}
	return nullptr;
}

TObjectPtr<UBehaviorTree> AUnit::BehaviorTree() const
{
	return UnitBehaviorTree_;
}

TWeakObjectPtr<AActor> AUnit::FollowedTarget() const
{
	return FollowedTarget_;
}

const TObjectPtr<UPath>& AUnit::Path() const
{
	return Path_;
}

void AUnit::SetFollowedTarget( TObjectPtr<AActor> followedTarget )
{
	FollowedTarget_ = followedTarget;
}

void AUnit::OnDeath()
{
	// When HP becomes 0

	if ( AttackComponent_ )
	{
		AttackComponent_->DeactivateSight();
	}

	if ( FollowComponent_ )
	{
		FollowComponent_->Deactivate();
	}

	if ( VisualMesh_ )
	{
		VisualMesh_->SetVisibility( false, true );
	}
	if ( CollisionComponent_ )
	{
		CollisionComponent_->SetCollisionEnabled( ECollisionEnabled::NoCollision );
	}
	SpawnDeathVFX();

	if ( ResolvedDeathDestroyDelay_ > 0.0f )
	{
		GetWorldTimerManager().SetTimer(
		    DeathTimerHandle_, this, &AUnit::FinalizeDestroy, ResolvedDeathDestroyDelay_, false
		);
	}
	else
	{
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

void AUnit::FinalizeDestroy()
{
	Destroy();
}

UNiagaraSystem* AUnit::GetHitVFX() const
{
	return ResolvedHitVFX_;
}

void AUnit::ResolveVFXDefaults()
{
	ResolvedDeathVFX_ = DeathVFX_;
	ResolvedHitVFX_ = HitVFX_;
	ResolvedDeathDestroyDelay_ = DeathDestroyDelay_ >= 0.0f ? DeathDestroyDelay_ : 1.0f;

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

				if ( DeathDestroyDelay_ < 0.0f && override->DeathDestroyDelay >= 0.0f )
				{
					ResolvedDeathDestroyDelay_ = override->DeathDestroyDelay;
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

			if ( DeathDestroyDelay_ < 0.0f && ResolvedDeathDestroyDelay_ <= 0.0f )
			{
				ResolvedDeathDestroyDelay_ = config->DefaultDeathDestroyDelay;
			}
		}
	}
}

void AUnit::FollowNextPathTarget()
{
	AdvancePathPointIndex();
	FollowPath();
}

bool AUnit::IsCloseToTarget() const
{
	if ( !FollowedTarget_.IsValid() || !UnitAIManager_.IsValid() || !IsValid( UnitAIManager_->PathPointsManager ) )
	{
		return false;
	}

	const float distanceSq = FVector::DistSquared( GetActorLocation(), FollowedTarget_->GetActorLocation() );
	const float radius = UnitAIManager_->PathPointsManager->PointReachRadius();
	const float radiusSq = radius * radius;
	return distanceSq < radiusSq;
}

void AUnit::SetPath( TObjectPtr<UPath> path )
{
	Path_ = path;
	PathPointIndex_ = 0;
}

void AUnit::SetUnitAIManager( TWeakObjectPtr<AUnitAIManager> unitAIManager )
{
	UnitAIManager_ = unitAIManager;
}

void AUnit::AdvancePathPointIndex()
{
	++PathPointIndex_;
}

void AUnit::SetPathPointIndex( int pathPointIndex )
{
	PathPointIndex_ = pathPointIndex;
}

void AUnit::FollowPath()
{
	if ( !Path_ )
	{
		UE_LOG( LogTemp, Error, TEXT( "Unit: no valid Path_. Cannot follow path" ) );
		FollowedTarget_ = nullptr;
		return;
	}

	if ( !UnitAIManager_.IsValid() )
	{
		UE_LOG( LogTemp, Error, TEXT( "Unit: no valid PathPointsManager_. Cannot follow path" ) );
		FollowedTarget_ = nullptr;
		return;
	}

	const TArray<FIntPoint>& pathPoints = Path_->GetPoints();
	if ( 0 > PathPointIndex_ || PathPointIndex_ >= pathPoints.Num() )
	{
		if ( UnitAIManager_->GoalActor.IsValid() )
		{
			FollowedTarget_ = UnitAIManager_->GoalActor;
		}
		else
		{
			UE_LOG( LogTemp, Error, TEXT( "Unit: PathPointIndex_ is out of range and no GoalActor specified" ) );
			FollowedTarget_ = nullptr;
		}
	}
	else
	{
		if ( IsValid( UnitAIManager_->PathPointsManager ))
		{
			FollowedTarget_ = UnitAIManager_->PathPointsManager->GetTargetPoint( pathPoints[PathPointIndex_] ).Get();
		}
	}
}

void AUnit::SetFollowedTarget( AActor* newTarget )
{
	FollowedTarget_ = newTarget;
}

TObjectPtr<USceneComponent> AUnit::VisualMesh()
{
	return VisualMesh_;
}

void AUnit::ChangeStats( FEnemyBuff* buff )
{
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
