// Fill out your copyright notice in the Description page of Project Settings.

#include "Lords_Frontiers/Public/Units/Unit.h"

#include "AI/EntityAIController.h"
#include "AI/UnitAIManager.h"
#include "Core/CoreManager.h"
#include "Transform/TransformableHandleUtils.h"
#include "Utilities/TraceChannelMappings.h"
#include "Waves/EnemyBuff.h"

#include "Components/CapsuleComponent.h"
#include "Components/FollowComponent.h"
#include "Kismet/GameplayStatics.h"

AUnit::AUnit()
{
	PrimaryActorTick.bCanEverTick = true;

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

	if ( const UCoreManager* core = UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<UCoreManager>() )
	{
		UnitAIManager_ = core->GetUnitAIManager();
	}

	VisualMesh_ = Cast<USceneComponent>( GetComponentByClass( UMeshComponent::StaticClass() ) );
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
	if ( AttackComponent_ )
	{
		AttackComponent_->Attack( hitActor );
	}
}

void AUnit::TakeDamage( float damage )
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
	// When HP becomes 0

	if ( AttackComponent_ )
	{
		AttackComponent_->DeactivateSight();
	}

	if ( FollowComponent_ )
	{
		FollowComponent_->Deactivate();
	}

	Destroy();
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
