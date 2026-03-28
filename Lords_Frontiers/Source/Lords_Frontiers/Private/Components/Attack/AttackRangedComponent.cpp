// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Attack/AttackRangedComponent.h"

#include "AI/Path/Path.h"
#include "AI/Path/PathPointsManager.h"
#include "AI/Path/PathTargetPoint.h"
#include "AI/UnitAIManager.h"
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

	if ( IEntity* entity = GetOwner<IEntity>() )
	{
		SightSphere_->SetupAttachment( GetOwner()->GetRootComponent() );
		SightSphere_->SetSphereRadius( entity->Stats().AttackRange() );
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
	IEntity* ownerEntity = GetOwner<IEntity>();
	IAttacker* ownerAttacker = GetOwner<IAttacker>();
	if ( !ownerEntity || !ownerAttacker )
	{
		UE_LOG( LogTemp, Error, TEXT( "UAttackRangedComponent::Attack: owner must be IEntity and IAttacker" ) );
		return;
	}

	if ( ownerEntity->Stats().OnCooldown() )
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

	if ( ProjectileClass_ )
	{
		const FActorSpawnParameters spawnParams;
		const auto projectile = world->SpawnActor<AProjectile>( ProjectileClass_, spawnTransform, spawnParams );
		projectile->Initialize( AttackTarget(), ownerEntity->Stats().AttackDamage(), ProjectileSpeed_ );
		projectile->Launch();
		ownerEntity->Stats().StartCooldown();
	}
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
	if ( IAttacker* ownerAttacker = GetOwner<IAttacker>() )
	{
		ownerAttacker->SetAttackTarget( nullptr );
	}
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

		if ( enemyPositionAttackable && CanSeeEnemy( actor ) )
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
	const IAttacker* ownerAttacker = GetOwner<IAttacker>();
	if ( ownerAttacker && !ownerAttacker->AttackTarget().IsValid() &&
	     GetOwner()->GetVelocity().Size() <= KINDA_SMALL_NUMBER )
	{
		// Super bad (unclear) code: buildings and standing units start attacking everything
		// Left in peace due to time constraints (laziness)
		// TODO: Separate components for unit and for building
		AttackFilter_ = EAttackFilter::Everything;
	}
	else
	{
		AttackFilter_ = EAttackFilter::WhatIsOnPath;
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
