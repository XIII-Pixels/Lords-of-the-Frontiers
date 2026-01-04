// Fill out your copyright notice in the Description page of Project Settings.

#include "Lords_Frontiers/Public/Units/Unit.h"

#include "Units/FollowComponent.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Transform/TransformableHandleUtils.h"
#include "Units/UnitAIController.h"
#include "Utilities/TraceChannelMappings.h"

AUnit::AUnit()
{
	CollisionComponent_ = CreateDefaultSubobject<UCapsuleComponent>( TEXT( "CapsuleCollision" ) );
	SetRootComponent( CollisionComponent_ );

	CollisionComponent_->SetCollisionObjectType( ECC_Entity );

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	UnitAIControllerClass_ = AUnitAIController::StaticClass();
	AIControllerClass = UnitAIControllerClass_;
}

void AUnit::OnConstruction(const FTransform& transform)
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
		FollowComponent_->UpdatedComponent = CollisionComponent_;
	}

	AttackComponent_ = FindComponentByClass<UAttackComponentBase>();
}

void AUnit::Tick(float deltaSeconds)
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

void AUnit::Attack(TObjectPtr<AActor> hitActor)
{
	if ( AttackComponent_ )
	{
		AttackComponent_->Attack( hitActor );
	}
}

void AUnit::TakeDamage(float damage)
{
	// UE_LOG( LogTemp, Display, TEXT( "Take damage" ) );
	if ( !Stats_.IsAlive() )
	{
		// UE_LOG( LogTemp, Display, TEXT( "Dead!" ) );
		return;
	}
	
	Stats_.ApplyDamage( damage );
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
