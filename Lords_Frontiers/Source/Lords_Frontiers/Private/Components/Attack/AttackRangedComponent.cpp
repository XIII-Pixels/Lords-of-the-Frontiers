// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Attack/AttackRangedComponent.h"

#include "AI/Path/Path.h"
#include "AI/Path/PathTargetPoint.h"
#include "Core/CoreManager.h"
#include "Entity.h"
#include "Grid/GridManager.h"
#include "Projectiles/Projectile.h"
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

	if ( GetOwner() )
	{
		SightSphere_->SetupAttachment( GetOwner()->GetRootComponent() );
		if ( OwnerEntity_ )
		{
			SightSphere_->SetSphereRadius( OwnerEntity_->Stats().AttackRange() );
		}
	}
}

void UAttackRangedComponent::BeginPlay()
{
	Super::BeginPlay();

	ActivateSight();
}

void UAttackRangedComponent::LookTick()
{
	ChooseAttackMode();
	Look();
}

void UAttackRangedComponent::Attack( TObjectPtr<AActor> hitActor )
{
	if ( !OwnerIsValid() )
	{
		return;
	}

	if ( OwnerEntity_->Stats().OnCooldown() )
	{
		return;
	}

	UWorld* world = GetOwner()->GetWorld();
	if ( !world )
	{
		return;
	}

	FTransform spawnTransform = GetOwner()->GetTransform();
	spawnTransform.AddToTranslation( ProjectileSpawnPosition_ );

	if ( IsValid( EnemyInSight_ ) && ProjectileClass_ )
	{
		const FActorSpawnParameters spawnParams;
		const auto projectile = world->SpawnActor<AProjectile>( ProjectileClass_, spawnTransform, spawnParams );
		projectile->Initialize( EnemyInSight(), OwnerEntity_->Stats().AttackDamage(), ProjectileSpeed_ );
		projectile->Launch();
		OwnerEntity_->Stats().StartCooldown();
	}
}

TObjectPtr<AActor> UAttackRangedComponent::EnemyInSight() const
{
	return EnemyInSight_;
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
	EnemyInSight_ = nullptr;
}

void UAttackRangedComponent::Look()
{
	if ( !OwnerIsValid() )
	{
		return;
	}

	EnemyInSight_ = nullptr;

	TArray<AActor*> overlappingActors;
	SightSphere_->GetOverlappingActors( overlappingActors, AActor::StaticClass() );

	float minDistance = -1.0f;
	for ( auto actor : overlappingActors )
	{
		if ( actor == GetOwner() )
		{
			continue;
		}

		if ( CanSeeEnemy( actor ) && ( AttackMode_ == EAttackMode::BeatEverything || EnemyIsOnPath( actor ) ) )
		{
			float distance = FVector::Distance( GetOwner()->GetActorLocation(), actor->GetActorLocation() );
			if ( !IsValid( EnemyInSight_ ) || distance < minDistance )
			{
				EnemyInSight_ = actor;
				minDistance = distance;
			}
		}
	}
}

void UAttackRangedComponent::ChooseAttackMode()
{
	if ( !IsValid( EnemyInSight_ ) && OwnerEntity_ &&
	     Cast<AActor>( OwnerEntity_ )->GetVelocity().Size() <= KINDA_SMALL_NUMBER )
	{
		AttackMode_ = EAttackMode::BeatEverything;
	}
	else
	{
		AttackMode_ = EAttackMode::Normal;
	}
}

bool UAttackRangedComponent::CanSeeEnemy( TObjectPtr<AActor> enemyActor ) const
{
	// it is assumed the actor is inside the sight sphere
	if ( !OwnerIsValid() )
	{
		return false;
	}

	const auto enemy = Cast<IEntity>( enemyActor );
	if ( enemy && enemy->Stats().IsAlive() && enemy->Team() != OwnerEntity_->Team() )
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

bool UAttackRangedComponent::EnemyIsOnPath( TObjectPtr<AActor> enemyActor ) const
{
	// Path destination is on path as well

	if ( !OwnerIsValid() )
	{
		return false;
	}

	const AUnit* unit = GetOwner<AUnit>();
	if ( !unit )
	{
		UE_LOG( LogTemp, Error, TEXT( "UAttackRangedComponent::EnemyIsOnPath: owner unit not found" ) );
		return false;
	}

	const AGridManager* grid = nullptr;
	if ( const UCoreManager* core = UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<UCoreManager>() )
	{
		grid = core->GetGridManager();
	}
	if ( !grid )
	{
		UE_LOG( LogTemp, Error, TEXT( "UAttackRangedComponent::EnemyIsOnPath: grid not found" ) );
		return false;
	}

	if ( const UPath* path = unit->Path() )
	{
		const FIntPoint enemyCoords = grid->GetCellCoords( enemyActor->GetActorLocation() );
		for ( const FIntPoint& point : path->GetPoints() )
		{
			// Path points storage will probably be reimplemented so this may be optimized in future
			if ( point == enemyCoords )
			{
				return true;
			}
		}
	}
	return false;
}
