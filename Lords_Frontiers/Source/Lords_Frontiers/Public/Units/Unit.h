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
 * Base class for all units in a game (implement units in blueprints)
 * Can move, attack and be attacked
 * Should be controlled by AI controller */
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API AUnit : public APawn, public IAttackable
{
	GENERATED_BODY()

public:
	AUnit();

	void OnConstruction(const FTransform& transform) override;

	virtual void BeginPlay() override;

	virtual void Tick(float deltaSeconds) override;

	void Attack(TObjectPtr<AActor> hitActor);

	// HP reduction
	void TakeDamage(float damage) override;
	
	ETeam Team() override;

	TObjectPtr<AActor> EnemyInSight() const;

	const TObjectPtr<UBehaviorTree>& BehaviorTree() const;

	const TObjectPtr<AActor>& Target() const;

protected:
	UPROPERTY( EditDefaultsOnly, Category = "Settings|AI" )
	TSubclassOf<AAIController> UnitAIControllerClass_;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|AI" )
	TObjectPtr<UBehaviorTree> UnitBehaviorTree_;

	UPROPERTY( EditAnywhere, Category = "Settings|AI" )
	TObjectPtr<AActor> FollowedTarget_;

	UPROPERTY( EditAnywhere, Category = "Settings")
	FEntityStats Stats_;

	UPROPERTY( VisibleDefaultsOnly, Category = "Settings" )
	TObjectPtr<UCapsuleComponent> CollisionComponent_;

	UPROPERTY( VisibleDefaultsOnly, Category = "Settings" )
	TObjectPtr<UUnitMovementComponent> MovementComponent_;

	// [Should be in Attack Component]
	// -----------------------------------------------------
	UPROPERTY( EditAnywhere, Category = "Settings")
	float LookForwardTimeInterval_ = 0.2f;
	
	UPROPERTY( VisibleAnywhere, Category = "Settings" )
	TObjectPtr<AActor> EnemyInSight_;

	// Look forward at given time intervals
	void SightTick();

	void LookForward();

	FTimerHandle SightTimerHandle_;
	// -----------------------------------------------------
};
