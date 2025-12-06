// Fill out your copyright notice in the Description page of Project Settings.

#include "Lords_Frontiers/Public/Units/Unit.h"

#include "Units/UnitMovementComponent.h"

#include "Components/CapsuleComponent.h"

AUnit::AUnit()
{
	// PivotPoint_ = CreateDefaultSubobject<USceneComponent>( TEXT( "PivotPoint" ) );
	// SetRootComponent( PivotPoint_ );

	CollisionComponent_ = CreateDefaultSubobject<UCapsuleComponent>( TEXT( "CapsuleCollision" ) );
	SetRootComponent( CollisionComponent_ );

	// float halfHeight = CollisionComponent_->GetUnscaledCapsuleHalfHeight();
	// CollisionComponent_->SetRelativeLocation( FVector( 0, 0, halfHeight ) );

	MovementComponent_ = CreateDefaultSubobject<UUnitMovementComponent>( TEXT( "UnitMovementComponent" ) );
	MovementComponent_->UpdatedComponent = CollisionComponent_;
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
	return BehaviorTree_;
}

const TObjectPtr<AActor>& AUnit::Target() const
{
	return Target_;
}
