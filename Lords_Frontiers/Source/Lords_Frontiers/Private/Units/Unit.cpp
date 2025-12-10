// Fill out your copyright notice in the Description page of Project Settings.

#include "Lords_Frontiers/Public/Units/Unit.h"

#include "Units/UnitMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "Units/UnitAIController.h"

AUnit::AUnit()
{
	CollisionComponent_ = CreateDefaultSubobject<UCapsuleComponent>( TEXT( "CapsuleCollision" ) );
	SetRootComponent( CollisionComponent_ );

	CollisionComponent_->SetCollisionProfileName( TEXT( "Pawn" ) );

	MovementComponent_ = CreateDefaultSubobject<UUnitMovementComponent>( TEXT( "UnitMovementComponent" ) );
	MovementComponent_->UpdatedComponent = CollisionComponent_;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	UnitAIControllerClass = AUnitAIController::StaticClass();
	AIControllerClass = UnitAIControllerClass;
}

void AUnit::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction( Transform );
	AIControllerClass = UnitAIControllerClass;
}

/** (Gregory-hub)
 * Attack someone or something
 * Calls IAttackable::TakeDamage on target */
void AUnit::Attack(TScriptInterface<IAttackable> target)
{
	// Probably should be done with some attack component, because some buildings can attack as well
	// Probably should use some attack manager, because it would be easier to fetch attack info
}

/** (Gregory-hub)
 * Decrease HP */
void AUnit::TakeDamage(float damage)
{
	// Stats.HP -= damage
}

const TObjectPtr<UBehaviorTree>& AUnit::BehaviorTree() const
{
	return UnitBehaviorTree;
}

const TObjectPtr<AActor>& AUnit::Target() const
{
	return FollowedTarget;
}
