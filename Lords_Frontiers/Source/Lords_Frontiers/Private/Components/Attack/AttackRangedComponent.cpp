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

	AActor* newTarget = GetOwner<IAttacker>()->AttackTarget().Get();

	bDidSeeTarget_ = prevTarget && ( prevTarget == newTarget );

	if ( prevTarget != newTarget )
	{
		OnAttackTargetChanged.Broadcast( prevTarget, newTarget );
	}
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
		AActor* target = ownerAttacker->AttackTarget().Get();
		OnBeforeAttackFire.Broadcast( target );
		FireSingleProjectile( target );
		PendingDamageBonusPercent_ = 0.f;
		ownerEntity->Stats().StartCooldown( gameTime );
		OnAttackFired.Broadcast( target );
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

	AActor* bestTarget = nullptr;
	float bestScore = 0.f;
	bool bHasBest = false;

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

		if ( !enemyPositionAttackable || !CanSeeEnemy( actor ) || !IsAttackable( actor ) )
		{
			continue;
		}

		float score = 0.f;
		switch ( TargetPriority_ )
		{
		case ETowerTargetPriority::LowestHP:
		{
			const IEntity* entity = Cast<IEntity>( actor );
			score = entity ? -static_cast<float>( entity->Stats().Health() ) : 0.f;
			break;
		}
		case ETowerTargetPriority::HighestHP:
		{
			const IEntity* entity = Cast<IEntity>( actor );
			score = entity ? static_cast<float>( entity->Stats().Health() ) : 0.f;
			break;
		}
		case ETowerTargetPriority::Closest:
		default:
			score = -FVector::Distance( GetOwner()->GetActorLocation(), actor->GetActorLocation() );
			break;
		}

		if ( !bHasBest || score > bestScore )
		{
			bestTarget = actor;
			bestScore = score;
			bHasBest = true;
		}
	}

	if ( bestTarget )
	{
		ownerAttacker->SetAttackTarget( bestTarget );
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

void UAttackRangedComponent::FireExtraProjectile( AActor* target, float damageMultiplier )
{
	UWorld* world = GetOwner() ? GetOwner()->GetWorld() : nullptr;
	if ( !world )
	{
		UE_LOG( LogTemp, Warning, TEXT( "FireExtraProjectile: world is null" ) );
		return;
	}
	if ( !target )
	{
		UE_LOG( LogTemp, Warning, TEXT( "FireExtraProjectile: target is null" ) );
		return;
	}
	if ( !ProjectileClass_ )
	{
		UE_LOG( LogTemp, Warning,
			TEXT( "FireExtraProjectile: ProjectileClass_ not set on %s" ),
			GetOwner() ? *GetOwner()->GetName() : TEXT( "null" ) );
		return;
	}

	const IEntity* ownerEntity = GetOwner<IEntity>();
	if ( !ownerEntity )
	{
		UE_LOG( LogTemp, Warning, TEXT( "FireExtraProjectile: owner is not IEntity" ) );
		return;
	}

	UProjectilePoolSubsystem* pool = world->GetSubsystem<UProjectilePoolSubsystem>();
	if ( !pool )
	{
		UE_LOG( LogTemp, Warning, TEXT( "FireExtraProjectile: ProjectilePoolSubsystem missing" ) );
		return;
	}

	ABaseProjectile* projectile = pool->AcquireProjectile( ProjectileClass_ );
	if ( !projectile )
	{
		UE_LOG( LogTemp, Warning, TEXT( "FireExtraProjectile: pool returned null projectile" ) );
		return;
	}

	const int32 scaledDamage = FMath::RoundToInt(
		static_cast<float>( ownerEntity->Stats().AttackDamage() ) * damageMultiplier );

	FVector spawnOffset = ProjectileSpawnPosition_;
	if ( AActor* owner = GetOwner() )
	{
		const FVector toTarget = ( target->GetActorLocation() - owner->GetActorLocation() ).GetSafeNormal2D();
		const FVector right = FVector::CrossProduct( toTarget, FVector::UpVector ).GetSafeNormal();
		const float jitter = FMath::FRandRange( -60.f, 60.f );
		spawnOffset += right * jitter;
	}

	const bool bInitialized = projectile->Initialize(
	    GetOwner(), target, scaledDamage, ProjectileSpeed_, spawnOffset,
	    ownerEntity->Stats().SplashRadius(), ownerEntity->Stats().AttackRange(), bProjectileTracksTarget_ );

	if ( !bInitialized )
	{
		UE_LOG( LogTemp, Warning,
			TEXT( "FireExtraProjectile: Initialize returned false (target invalid?)" ) );
		pool->ReturnProjectile( projectile );
		return;
	}

	UE_LOG( LogTemp, Log,
		TEXT( "FireExtraProjectile: %s → %s for damage %d" ),
		*GetOwner()->GetName(), *target->GetName(), scaledDamage );

	OnBeforeAttackFire.Broadcast( target );
	OnAttackFired.Broadcast( target );
}

void UAttackRangedComponent::FireShrapnel(
	const FVector& spawnLocation, int32 count, float damageMultiplier,
	float spreadDegrees, float range, float speed, float splashRadius,
	TSubclassOf<ABaseProjectile> overrideClass )
{
	if ( count <= 0 || range <= 0.f || speed <= 0.f )
	{
		return;
	}

	UWorld* world = GetOwner() ? GetOwner()->GetWorld() : nullptr;
	if ( !world )
	{
		return;
	}

	TSubclassOf<ABaseProjectile> projectileClass = overrideClass ? overrideClass : ProjectileClass_;
	if ( !projectileClass )
	{
		UE_LOG( LogTemp, Warning,
			TEXT( "FireShrapnel: no projectile class on %s" ),
			GetOwner() ? *GetOwner()->GetName() : TEXT( "null" ) );
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

	const int32 scaledDamage = FMath::RoundToInt(
		static_cast<float>( ownerEntity->Stats().AttackDamage() ) * damageMultiplier );

	const float spread = FMath::Clamp( spreadDegrees, 0.f, 360.f );
	const float baseYaw = FMath::FRandRange( 0.f, 360.f );

	for ( int32 i = 0; i < count; ++i )
	{
		ABaseProjectile* projectile = pool->AcquireProjectile( projectileClass );
		if ( !projectile )
		{
			continue;
		}

		float yawDeg = 0.f;
		if ( count == 1 )
		{
			yawDeg = baseYaw;
		}
		else if ( FMath::IsNearlyEqual( spread, 360.f ) )
		{
			yawDeg = baseYaw + ( 360.f / count ) * i;
		}
		else
		{
			const float step = spread / ( count - 1 );
			yawDeg = baseYaw - spread * 0.5f + step * i;
		}

		const FVector direction = FRotator( 0.f, yawDeg, 0.f ).Vector();

		const bool bInitialized = projectile->InitializeDirectional(
			GetOwner(), spawnLocation, direction, scaledDamage, speed, range, splashRadius );

		if ( !bInitialized )
		{
			pool->ReturnProjectile( projectile );
		}
	}
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

	float damageFloat = static_cast<float>( ownerEntity->Stats().AttackDamage() );

	if ( !FMath::IsNearlyZero( PendingDamageBonusPercent_ ) )
	{
		damageFloat *= ( 1.f + PendingDamageBonusPercent_ / 100.f );
	}

	const int32 critChance = ownerEntity->Stats().CritChance();
	if ( critChance > 0 && FMath::RandRange( 1, 100 ) <= critChance )
	{
		const float critMultiplier = 1.f + static_cast<float>( ownerEntity->Stats().CritDamageBonus() ) / 100.f;
		damageFloat *= critMultiplier;
	}

	const int32 finalDamage = FMath::RoundToInt( damageFloat );

	const bool bInitialized = projectile->Initialize(
	    GetOwner(), target.Get(), finalDamage, ProjectileSpeed_, ProjectileSpawnPosition_,
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
		OnBeforeAttackFire.Broadcast( target.Get() );
		FireSingleProjectile( target );
		PendingDamageBonusPercent_ = 0.f;
		OnAttackFired.Broadcast( target.Get() );
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
