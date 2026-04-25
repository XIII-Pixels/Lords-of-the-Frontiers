// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Attack/AttackRangedComponent.h"

#include "AI/Path/Path.h"
#include "AI/Path/PathPointsManager.h"
#include "AI/Path/PathTargetPoint.h"
#include "AI/UnitAIManager.h"
#include "Core/CoreManager.h"
#include "Core/Subsystems/ProjectilePoolSubsystem/ProjectilePoolSubsystem.h"
#include "Entity.h"
#include "Projectiles/BaseProjectile.h"
#include "Units/Unit.h"
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

	if ( IEntity* entity = GetOwner<IEntity>() )
	{
		SightSphere_->SetupAttachment( GetOwner()->GetRootComponent() );
		SightSphere_->SetSphereRadius( entity->Stats().AttackRange() );
	}
}

void UAttackRangedComponent::BeginPlay()
{
	Super::BeginPlay();

	ChooseAttackMode();
	ActivateSight();
}

void UAttackRangedComponent::LookTick()
{
	AActor* prevTarget = GetOwner<IAttacker>()->AttackTarget().Get();

	Look();

	bDidSeeTarget_ = prevTarget && ( prevTarget == GetOwner<IAttacker>()->AttackTarget() );
}

void UAttackRangedComponent::Attack( TObjectPtr<AActor> hitActor )
{
	IEntity* ownerEntity = GetOwner<IEntity>();
	IAttacker* ownerAttacker = GetOwner<IAttacker>();
	if ( !ownerEntity || !ownerAttacker )
	{
		UE_LOG( LogTemp, Error, TEXT( "UAttackRangedComponent::Attack: owner must be IEntity and IAttacker" ) );
		return;
	}

	const float gameTime = GetWorld()->GetTimeSeconds();

	if ( ownerEntity->Stats().OnCooldown( gameTime ) || bBurstInProgress_ )
	{
		return;
	}

	if ( !ownerAttacker->AttackTarget().IsValid() || !ProjectileClass_ )
	{
		return;
	}

	const int32 burstCount = ownerEntity->Stats().BurstCount();
	if ( burstCount <= 1 )
	{
		FireSingleProjectile( ownerAttacker->AttackTarget().Get() );
		ownerEntity->Stats().StartCooldown( gameTime );
		return;
	}

	BurstTargets_.Empty();

	if ( ownerEntity->Stats().BurstTargetMode() == EBurstTargetMode::SameTarget )
	{
		for ( int32 i = 0; i < burstCount; ++i )
		{
			BurstTargets_.Add( ownerAttacker->AttackTarget() );
		}
	}
	else
	{
		TArray<TObjectPtr<AActor>> uniqueTargets = FindNeighborTargets( burstCount );
		if ( uniqueTargets.Num() == 0 )
		{
			return;
		}

		for ( int32 i = 0; i < burstCount; ++i )
		{
			BurstTargets_.Add( uniqueTargets[i % uniqueTargets.Num()] );
		}
	}

	bBurstInProgress_ = true;
	CurrentBurstIndex_ = 0;
	FireNextBurstShot();
}

void UAttackRangedComponent::ActivateSight()
{
	GetWorld()->GetTimerManager().SetTimer(
	    SightTimerHandle_, this, &UAttackRangedComponent::LookTick, LookForwardTimeInterval_, true
	);
}

void UAttackRangedComponent::DeactivateSight()
{
	GetWorld()->GetTimerManager().ClearTimer( SightTimerHandle_ );
	GetWorld()->GetTimerManager().ClearTimer( BurstTimerHandle_ );
	bBurstInProgress_ = false;
	BurstTargets_.Empty();
	if ( IAttacker* ownerAttacker = GetOwner<IAttacker>() )
	{
		ownerAttacker->SetAttackTarget( nullptr );
	}
}

bool UAttackRangedComponent::DidSeeTargetLastTick()
{
	return bDidSeeTarget_;
}

void UAttackRangedComponent::Look()
{
	IAttacker* ownerAttacker = GetOwner<IAttacker>();
	if ( !ownerAttacker )
	{
		return;
	}

	const UPathPointsManager* pathPointsManager = nullptr;
	if ( const UCoreManager* coreManager = UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<UCoreManager>() )
	{
		if ( const AUnitAIManager* aiManager = coreManager->GetUnitAIManager() )
		{
			pathPointsManager = aiManager->PathPointsManager();
		}
	}

	ownerAttacker->SetAttackTarget( nullptr );

	TArray<AActor*> overlappingActors;
	SightSphere_->GetOverlappingActors( overlappingActors, AActor::StaticClass() );

	float minDistance = -1.0f;
	for ( auto actor : overlappingActors )
	{
		if ( actor == GetOwner() )
		{
			continue;
		}

		bool enemyPositionAttackable = false;
		if ( AttackFilter_ == EAttackFilter::Everything )
		{
			enemyPositionAttackable = true;
		}
		else if ( AttackFilter_ == EAttackFilter::WhatIsOnPath && pathPointsManager )
		{
			if ( const AUnit* unit = GetOwner<AUnit>() )
			{
				enemyPositionAttackable = pathPointsManager->ActorIsOnPath( actor, unit->Path() );
			}
		}

		if ( enemyPositionAttackable && CanSeeEnemy( actor ) && IsAttackable( actor ) )
		{
			const float distance = FVector::Distance( GetOwner()->GetActorLocation(), actor->GetActorLocation() );
			if ( !ownerAttacker->AttackTarget().IsValid() || distance < minDistance )
			{
				ownerAttacker->SetAttackTarget( actor );
				minDistance = distance;
			}
		}
	}
}

void UAttackRangedComponent::ChooseAttackMode()
{
	// Bad: hard linked to ABuilding and AUnit
	// TODO: Separate components for unit and for building
	if ( GetOwner<AUnit>() )
	{
		AttackFilter_ = EAttackFilter::WhatIsOnPath;
	}
	else
	{
		AttackFilter_ = EAttackFilter::Everything;
	}
}

bool UAttackRangedComponent::CanSeeEnemy( TObjectPtr<AActor> enemyActor ) const
{
	// it is assumed the actor is inside the sight sphere
	const IEntity* ownerEntity = GetOwner<IEntity>();
	if ( !ownerEntity )
	{
		return false;
	}

	const auto enemy = Cast<IEntity>( enemyActor );
	if ( enemy && enemy->Stats().IsAlive() && enemy->Team() != ownerEntity->Team() )
	{
		if ( !bCanAttackBackward_ ) // look in half-circle in front of unit
		{
			const float dot = FVector::DotProduct(
			    enemyActor->GetActorLocation() - GetOwner()->GetActorLocation(), GetOwner()->GetActorForwardVector()
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

void UAttackRangedComponent::FireSingleProjectile( TWeakObjectPtr<AActor> target ) const
{
	UWorld* world = GetOwner()->GetWorld();
	if ( !world || !target.IsValid() || !ProjectileClass_ )
	{
		return;
	}

	const IEntity* ownerEntity = GetOwner<IEntity>();
	if ( !ownerEntity )
	{
		return;
	}

	UProjectilePoolSubsystem* pool = world->GetSubsystem<UProjectilePoolSubsystem>();
	if ( !pool )
	{
		return;
	}

	ABaseProjectile* projectile = pool->AcquireProjectile( ProjectileClass_ );
	if ( !projectile )
	{
		return;
	}

	const bool bInitialized = projectile->Initialize(
	    GetOwner(), target.Get(), ownerEntity->Stats().AttackDamage(), ProjectileSpeed_, ProjectileSpawnPosition_,
	    ownerEntity->Stats().SplashRadius(), ownerEntity->Stats().AttackRange(), bProjectileTracksTarget_
	);

	if ( !bInitialized )
	{
		pool->ReturnProjectile( projectile );
	}
}

TArray<TObjectPtr<AActor>> UAttackRangedComponent::FindNeighborTargets( int32 count ) const
{
	TArray<TObjectPtr<AActor>> result;
	TArray<AActor*> overlappingActors;
	SightSphere_->GetOverlappingActors( overlappingActors, AActor::StaticClass() );

	struct FActorDistance
	{
		TObjectPtr<AActor> Actor;
		float Distance;
	};
	TArray<FActorDistance> candidates;

	const FVector ownerLocation = GetOwner()->GetActorLocation();
	for ( auto actor : overlappingActors )
	{
		if ( actor == GetOwner() )
		{
			continue;
		}
		if ( CanSeeEnemy( actor ) && IsAttackable( actor ) )
		{
			float distance = FVector::DistSquared( ownerLocation, actor->GetActorLocation() );
			candidates.Add( { actor, distance } );
		}
	}
	const int32 resultCount = FMath::Min( count, candidates.Num() );
	std::partial_sort(
	    candidates.GetData(), candidates.GetData() + resultCount, candidates.GetData() + candidates.Num(),
	    []( const FActorDistance& a, const FActorDistance& b ) { return a.Distance < b.Distance; }
	);

	for ( int32 i = 0; i < resultCount; ++i )
	{
		result.Add( candidates[i].Actor );
	}

	return result;
}

void UAttackRangedComponent::FireNextBurstShot()
{
	IEntity* ownerEntity = GetOwner<IEntity>();
	const IAttacker* ownerAttacker = GetOwner<IAttacker>();
	if ( !ownerEntity || !ownerAttacker || !bBurstInProgress_ )
	{
		bBurstInProgress_ = false;
		BurstTargets_.Empty();
		return;
	}

	if ( CurrentBurstIndex_ >= BurstTargets_.Num() )
	{
		ownerEntity->Stats().StartCooldown( GetWorld()->GetTimeSeconds() );
		bBurstInProgress_ = false;
		BurstTargets_.Empty();
		return;
	}

	TWeakObjectPtr<AActor> target = BurstTargets_[CurrentBurstIndex_];

	if ( !target.IsValid() || !Cast<IEntity>( target ) || !Cast<IEntity>( target )->Stats().IsAlive() )
	{
		target = ownerAttacker->AttackTarget();
	}

	if ( target.IsValid() )
	{
		FireSingleProjectile( target );
	}
	++CurrentBurstIndex_;
	if ( CurrentBurstIndex_ >= BurstTargets_.Num() )
	{
		ownerEntity->Stats().StartCooldown( GetWorld()->GetTimeSeconds() );
		bBurstInProgress_ = false;
		BurstTargets_.Empty();
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(
		    BurstTimerHandle_, this, &UAttackRangedComponent::FireNextBurstShot, ownerEntity->Stats().BurstDelay(),
		    false
		);
	}
}
