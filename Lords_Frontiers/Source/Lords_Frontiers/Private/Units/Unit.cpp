// Fill out your copyright notice in the Description page of Project Settings.

#include "Lords_Frontiers/Public/Units/Unit.h"

#include "Transform/TransformableHandleUtils.h"
#include "Components/FollowComponent.h"
#include "Units/UnitAIController.h"
#include "Utilities/TraceChannelMappings.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

AUnit::AUnit()
{
	CollisionComponent_ = CreateDefaultSubobject<UCapsuleComponent>( TEXT( "CapsuleCollision" ) );
	SetRootComponent( CollisionComponent_ );

	CollisionComponent_->SetCollisionObjectType( ECC_Entity );

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	UnitAIControllerClass_ = AUnitAIController::StaticClass();
}

void AUnit::OnConstruction( const FTransform& transform )
{
	Super::OnConstruction( transform );
	AIControllerClass = UnitAIControllerClass_;
}

void AUnit::BeginPlay()
{
	Super::BeginPlay();

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
		    LogTemp, Error, TEXT( "Number of unit attack component is not equal to 1 (number: %d)" ),
		    attackComponents.Num()
		);
	}
}

void AUnit::Tick( float deltaSeconds )
{
	Super::Tick( deltaSeconds );
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

TObjectPtr<AActor> AUnit::FollowedTarget() const
{
	return FollowedTarget_;
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
