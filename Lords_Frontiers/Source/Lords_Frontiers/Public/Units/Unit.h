// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"

#include "Attackable.h"

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

	// Attack and damage
	virtual void Attack(TScriptInterface<IAttackable> target);

	void TakeDamage(float damage) override;

	// Getters
	const TObjectPtr<UBehaviorTree>& BehaviorTree() const;

	const TObjectPtr<AActor>& Target() const;

	void OnConstruction(const FTransform& Transform) override;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|AI" )
	TSubclassOf<AAIController> UnitAIControllerClass;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|AI" )
	TObjectPtr<UBehaviorTree> UnitBehaviorTree;

	UPROPERTY( EditAnywhere, Category = "Settings|AI" )
	TObjectPtr<AActor> FollowedTarget;

protected:
	UPROPERTY( VisibleDefaultsOnly )
	TObjectPtr<UCapsuleComponent> CollisionComponent_;

	UPROPERTY( VisibleDefaultsOnly )
	TObjectPtr<UUnitMovementComponent> MovementComponent_;
};
