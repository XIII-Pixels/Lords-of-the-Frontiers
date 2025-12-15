// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"

#include "Attackable.h"
#include "EntityStats.h"

#include "GameFramework/Pawn.h"
#include "Unit.generated.h"

class UCapsuleComponent;
class UBehaviorTree;
class UUnitMovementComponent;

/** (Gregory-hub)
 * Base class for all units in a game
 * Can move and attack
 * Should be controlled by AI controller */
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API AUnit : public APawn, public IAttackable
{
	GENERATED_BODY()

public:
	AUnit();

	void OnConstruction(const FTransform& transform) override;

	virtual void Tick(float deltaSeconds) override;

	// Attacks the closest attackable target in front of unit
	void AttackForward();

	// Calls IAttackable::TakeDamage on target
	void DealDamage(TScriptInterface<IAttackable> target);

	void TakeDamage(float damage) override;

	const TObjectPtr<UBehaviorTree>& BehaviorTree() const;

	const TObjectPtr<AActor>& Target() const;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|AI" )
	TSubclassOf<AAIController> UnitAIControllerClass;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|AI" )
	TObjectPtr<UBehaviorTree> UnitBehaviorTree;

	UPROPERTY( EditAnywhere, Category = "Settings|AI" )
	TObjectPtr<AActor> FollowedTarget;

	UPROPERTY( EditAnywhere, Category = "Settings")
	FEntityStats Stats;

protected:
	UPROPERTY( VisibleDefaultsOnly )
	TObjectPtr<UCapsuleComponent> CollisionComponent_;

	UPROPERTY( VisibleDefaultsOnly )
	TObjectPtr<UUnitMovementComponent> MovementComponent_;
};
