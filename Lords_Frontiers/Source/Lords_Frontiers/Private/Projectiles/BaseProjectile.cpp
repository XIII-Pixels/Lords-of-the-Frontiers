// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectiles/BaseProjectile.h"

#include "Core/CoreManager.h"
#include "VFX/EntityVFXConfig.h"
#include "Core/Subsystems/ProjectilePoolSubsystem/ProjectilePoolSubsystem.h"
#include "Core/Subsystems/SessionLogger/DamageEvent.h"
#include "DrawDebugHelpers.h"
#include "Entity.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Utilities/TraceChannelMappings.h"

#include "Components/MeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/OverlapResult.h"
#include "TimerManager.h"

ABaseProjectile::ABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	PrimaryActorTick.bStartWithTickEnabled = false;

	CollisionComp_ = CreateDefaultSubobject<USphereComponent>( TEXT( "CollisionSphere" ) );
	RootComponent = CollisionComp_;

	CollisionComp_->SetCollisionEnabled( ECollisionEnabled::QueryOnly );
	CollisionComp_->SetCollisionObjectType( ECC_InvisibleVolume );
	CollisionComp_->SetCollisionResponseToAllChannels( ECR_Ignore );
	CollisionComp_->SetCollisionResponseToChannel( ECC_Entity, ECR_Overlap );
	CollisionComp_->SetGenerateOverlapEvents( true );

	CollisionComp_->OnComponentBeginOverlap.AddDynamic( this, &ABaseProjectile::OnCollisionStart );
}

void ABaseProjectile::BeginDeactivation()
{
	if ( bIsPendingReturn_ )
	{
		return;
	}
	bIsPendingReturn_ = true;

	bIsActive_ = false;
	SetActorEnableCollision( false );
	SetActorTickEnabled( false );

	if ( UWorld* world = GetWorld() )
	{
		world->GetTimerManager().ClearTimer( LifetimeTimerHandle );
	}

	TArray<UMeshComponent*> meshes;
	GetComponents<UMeshComponent>( meshes );
	for ( UMeshComponent* mesh : meshes )
	{
		if ( mesh )
		{
			mesh->SetVisibility( false, false );
		}
	}

	TArray<UNiagaraComponent*> niagaraComps;
	GetComponents<UNiagaraComponent>( niagaraComps );
	for ( UNiagaraComponent* niagara : niagaraComps )
	{
		if ( !niagara )
		{
			continue;
		}
		niagara->SetAutoDestroy( false );
		niagara->Deactivate();
	}

	if ( UWorld* world = GetWorld() )
	{
		if ( TrailFadeDuration_ > 0.0f )
		{
			world->GetTimerManager().SetTimer(
			    TrailFinishTimerHandle, this, &ABaseProjectile::FinalizeDeactivation, TrailFadeDuration_, false
			);
		}
		else
		{
			FinalizeDeactivation();
		}
	}
	else
	{
		FinalizeDeactivation();
	}
}

void ABaseProjectile::FinalizeDeactivation()
{
	if ( UWorld* world = GetWorld() )
	{
		world->GetTimerManager().ClearTimer( TrailFinishTimerHandle );
	}

	TArray<UNiagaraComponent*> niagaraComps;
	GetComponents<UNiagaraComponent>( niagaraComps );
	for ( UNiagaraComponent* niagara : niagaraComps )
	{
		if ( niagara )
		{
			niagara->DeactivateImmediate();
		}
	}

	TArray<UMeshComponent*> meshes;
	GetComponents<UMeshComponent>( meshes );
	for ( UMeshComponent* mesh : meshes )
	{
		if ( mesh )
		{
			mesh->SetVisibility( true, false );
		}
	}

	SetActorHiddenInGame( true );
	SetActorLocation( PooledLocation );

	Target_ = nullptr;
	SetInstigator( nullptr );
	SetOwner( nullptr );

	FlightProgress_ = 0.0f;
	FlightDuration_ = 0.0f;

	MaxRange_ = 0.0f;
	ArcScale_ = 1.0f;

	GroundZ_ = 0.0f;
	bTrackTarget_ = GetDefault<ABaseProjectile>( GetClass() )->bTrackTarget_;

	bIsPendingReturn_ = false;

	if ( UWorld* world = GetWorld() )
	{
		if ( UProjectilePoolSubsystem* pool = world->GetSubsystem<UProjectilePoolSubsystem>() )
		{
			pool->FinalizeReturn( this );
		}
	}
}

void ABaseProjectile::DeactivateToPool()
{
	BeginDeactivation();
	if ( UWorld* world = GetWorld() )
	{
		world->GetTimerManager().ClearTimer( TrailFinishTimerHandle );
	}
	FinalizeDeactivation();
}

void ABaseProjectile::ActivateFromPool()
{
	bIsPendingReturn_ = false;

	StartLocation_ = GetActorLocation();

	if ( Target_.IsValid() )
	{
		TargetLocation_ = Target_->GetActorLocation();
	}
	else
	{
		TargetLocation_ = StartLocation_ + GetActorForwardVector() * 500.0f;
	}

	const float distance = FVector::Dist( StartLocation_, TargetLocation_ );
	FlightDuration_ = FMath::Max( distance / Speed_, 0.1f );
	FlightProgress_ = 0.0f;

	ArcScale_ = ( MaxRange_ > 0.0f ) ? FMath::Clamp( distance / MaxRange_, 0.0f, 1.0f ) : 1.0f;
	bIsActive_ = true;

	SetActorHiddenInGame( false );
	SetActorTickEnabled( true );
	SetActorEnableCollision( true );

	TArray<UMeshComponent*> meshes;
	GetComponents<UMeshComponent>( meshes );
	for ( UMeshComponent* mesh : meshes )
	{
		if ( mesh )
		{
			mesh->SetVisibility( true, false );
		}
	}

	TArray<UNiagaraComponent*> niagaraComps;
	GetComponents<UNiagaraComponent>( niagaraComps );
	for ( UNiagaraComponent* niagara : niagaraComps )
	{
		if ( niagara )
		{
			niagara->SetAutoDestroy( false );
			niagara->Activate( true );
		}
	}

	GetWorld()->GetTimerManager().SetTimer(
	    LifetimeTimerHandle, this, &ABaseProjectile::OnLifetimeExpired, MaxLifetime, false
	);
}

bool ABaseProjectile::Initialize(
    AActor* inInstigator, TWeakObjectPtr<AActor> inTarget, int inDamage, float inSpeed, const FVector& spawnOffset,
    float inSplashRadius, float inMaxRange, bool bTrackTarget
)
{
	if ( !IsValid( inInstigator ) || !inTarget.IsValid() )
	{
		return false;
	}

	Target_ = inTarget;
	Damage_ = FMath::Max( inDamage, 0 );
	Speed_ = FMath::Max( inSpeed, 0.0f );
	SplashRadius_ = FMath::Max( inSplashRadius, 0.f );
	MaxRange_ = FMath::Max( inMaxRange, 0.0f );
	bTrackTarget_ = bTrackTarget;
	SetInstigator( inInstigator->GetInstigator() );
	SetOwner( inInstigator );

	FHitResult GroundHit;
	const FVector TargetPos = inTarget->GetActorLocation();

	FCollisionQueryParams GroundTraceParams;
	GroundTraceParams.AddIgnoredActor( this );
	GroundTraceParams.AddIgnoredActor( inInstigator );
	GroundTraceParams.AddIgnoredActor( inTarget.Get() );

	if ( GetWorld()->LineTraceSingleByChannel(
	         GroundHit, TargetPos, TargetPos - FVector( 0, 0, 1000.f ), ECC_Visibility, GroundTraceParams
	     ) )
	{
		GroundZ_ = GroundHit.ImpactPoint.Z;
	}
	else
	{
		GroundZ_ = TargetPos.Z;
	}

	const FVector spawnLocation =
	    inInstigator->GetActorLocation() +
	    spawnOffset.RotateAngleAxis( inInstigator->GetActorRotation().Yaw, FVector::UpVector );

	const FVector toTarget = inTarget->GetActorLocation() - spawnLocation;

	SetActorLocationAndRotation( spawnLocation, toTarget.Rotation() );

	ActivateFromPool();

	return true;
}

void ABaseProjectile::Tick( float deltaTime )
{
	AActor::Tick( deltaTime );
	if ( !bIsActive_ )
	{
		return;
	}

	if ( bTrackTarget_ && IsValid( Target_.Get() ) )
	{
		TargetLocation_ = Target_->GetActorLocation();

		const IEntity* entity = Cast<IEntity>( Target_.Get() );
		if ( entity && !entity->Stats().IsAlive() )
		{
			Target_ = nullptr;
		}
	}

	FlightProgress_ += deltaTime / FlightDuration_;

	if ( FlightProgress_ >= 1.0f )
	{
		const FVector ImpactLocation( TargetLocation_.X, TargetLocation_.Y, GroundZ_ );
		SetActorLocation( ImpactLocation );

		if ( bTrackTarget_ && Target_.IsValid() )
		{
			SpawnHitVFX( Target_.Get(), ImpactLocation );

			if ( SplashRadius_ > 0.0f )
			{
				if ( UNiagaraSystem* groundVFX = GetProjectileImpactVFX( true ) )
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					    GetWorld(), groundVFX, ImpactLocation, GetActorRotation()
					);
				}
			}

			DealDamage( Target_.Get() );
		}
		else
		{
			SpawnHitVFX( nullptr, ImpactLocation );
			DealDamage( nullptr );
		}

		ReturnToPool();
		return;
	}
	const float t = FlightProgress_;
	const FVector previousLocation = GetActorLocation();

	FVector newLocation;
	newLocation.X = FMath::Lerp( StartLocation_.X, TargetLocation_.X, t );
	newLocation.Y = FMath::Lerp( StartLocation_.Y, TargetLocation_.Y, t );
	newLocation.Z =
	    FMath::Lerp( StartLocation_.Z, TargetLocation_.Z, t ) + ArcHeight_ * ArcScale_ * 4.0f * t * ( 1.0f - t );

	const FVector moveDirection = newLocation - previousLocation;
	const FRotator newRotation = moveDirection.IsNearlyZero() ? GetActorRotation() : moveDirection.Rotation();

	SetActorLocationAndRotation( newLocation, newRotation );
}

void ABaseProjectile::OnCollisionStart(
    UPrimitiveComponent* overlappedComp, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex,
    bool bFromSweep, const FHitResult& sweepResult
)
{
	if ( !bIsActive_ )
	{
		return;
	}

	if ( Target_.IsValid() && otherActor == Target_ )
	{
		SpawnHitVFX( otherActor, GetActorLocation() );
		DealDamage( otherActor );
		ReturnToPool();
	}
	else if ( !bTrackTarget_ || !Target_.IsValid() )
	{
		IEntity* enemy = Cast<IEntity>( otherActor );
		if ( !enemy || !enemy->Stats().IsAlive() )
		{
			return;
		}

		IEntity* ownerEntity = Cast<IEntity>( GetInstigator() );
		if ( ownerEntity && enemy->Team() != ownerEntity->Team() )
		{
			SpawnHitVFX( otherActor, GetActorLocation() );
			DealDamage( otherActor );
			ReturnToPool();
		}
	}
}

void ABaseProjectile::DealDamage( AActor* hitActor ) const
{
	if ( hitActor )
	{
		if ( IEntity* target = Cast<IEntity>( hitActor ) )
		{
			if ( target->Stats().IsAlive() )
			{
				target->TakeDamage( Damage_ );
				FDamageEvents::OnDamageDealt.Broadcast( GetOwner(), hitActor, Damage_, false );
			}
		}
	}

	if ( SplashRadius_ <= 0.0f )
	{
		return;
	}

#if ENABLE_DRAW_DEBUG
	DrawDebugSphere( GetWorld(), GetActorLocation(), SplashRadius_, 16, FColor::Red, false, 2.0f, 0, 2.0f );
#endif

	TArray<FOverlapResult> overlaps;
	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor( this );
	queryParams.AddIgnoredActor( GetOwner() );

	const bool bHasOverlaps = GetWorld()->OverlapMultiByChannel(
	    overlaps, GetActorLocation(), FQuat::Identity, ECC_Entity, FCollisionShape::MakeSphere( SplashRadius_ ),
	    queryParams
	);

	if ( !bHasOverlaps )
	{
		return;
	}

	IEntity* ownerEntity = Cast<IEntity>( GetInstigator() );
	if ( !ownerEntity )
	{
		return;
	}

	TSet<AActor*> damagedActors;
	damagedActors.Add( hitActor );

	for ( const FOverlapResult& overlap : overlaps )
	{
		if ( const auto* comp = overlap.GetComponent() )
		{
			if ( comp->GetCollisionObjectType() != ECC_Entity )
			{
				continue;
			}
		}

		AActor* overlapActor = overlap.GetActor();
		if ( !overlapActor )
		{
			continue;
		}

		if ( damagedActors.Contains( overlapActor ) )
		{
			continue;
		}

		IEntity* enemy = Cast<IEntity>( overlapActor );
		if ( !enemy )
		{
			continue;
		}

		if ( enemy->Stats().IsAlive() && enemy->Team() != ownerEntity->Team() )
		{
			enemy->TakeDamage( Damage_ );
			FDamageEvents::OnDamageDealt.Broadcast( GetOwner(), overlapActor, Damage_, true );
			damagedActors.Add( overlapActor );
		}
	}
}

void ABaseProjectile::ReturnToPool()
{
	if ( !bIsActive_ )
	{
		return;
	}

	if ( UProjectilePoolSubsystem* pool = GetWorld()->GetSubsystem<UProjectilePoolSubsystem>() )
	{
		pool->ReturnProjectile( this );
	}
}

void ABaseProjectile::OnLifetimeExpired()
{
	ReturnToPool();
}

UNiagaraSystem* ABaseProjectile::GetProjectileImpactVFX( bool bIsGroundHit ) const
{
	UCoreManager* core = UCoreManager::Get( this );
	if ( !core )
	{
		return nullptr;
	}

	UEntityVFXConfig* vfxConfig = core->GetEntityVFXConfig();
	if ( !vfxConfig )
	{
		return nullptr;
	}

	if ( const FProjectileImpactVFX* override = vfxConfig->ProjectileVFXOverrides.Find( ProjectileType ) )
	{
		UNiagaraSystem* vfx = bIsGroundHit ? override->GroundImpactVFX.Get() : override->ImpactVFX.Get();
		if ( vfx )
		{
			return vfx;
		}

		vfx = bIsGroundHit ? override->ImpactVFX.Get() : override->GroundImpactVFX.Get();
		if ( vfx )
		{
			return vfx;
		}
	}

	return vfxConfig->DefaultProjectileImpactVFX;
}

void ABaseProjectile::SpawnHitVFX( AActor* hitActor, const FVector& impactLocation ) const
{
	const bool bIsGroundHit = ( hitActor == nullptr );
	UNiagaraSystem* hitVFX = GetProjectileImpactVFX( bIsGroundHit );

	if ( !hitVFX && hitActor )
	{
		if ( IEntity* entity = Cast<IEntity>( hitActor ) )
		{
			hitVFX = entity->GetHitVFX();
		}
	}

	if ( !hitVFX )
	{
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation( GetWorld(), hitVFX, impactLocation, GetActorRotation() );
}
