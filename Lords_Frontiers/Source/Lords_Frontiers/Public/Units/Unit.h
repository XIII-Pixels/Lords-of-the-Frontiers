// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "Attackable.h"
#include "EntityStats.h"
#include "Components/Attack/AttackComponent.h"

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "Unit.generated.h"

class UCapsuleComponent;
class UBehaviorTree;
class UFollowComponent;

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

	virtual void OnConstruction( const FTransform& transform ) override;

	virtual void BeginPlay() override;

	virtual void Tick( float deltaSeconds ) override;

	void StartFollowing();

	void StopFollowing();

	void Attack( TObjectPtr<AActor> hitActor );

	virtual void TakeDamage( float damage ) override;

	// Getters and setters

	FEntityStats& Stats();

	virtual ETeam Team() override;

	TObjectPtr<AActor> EnemyInSight() const;

	TObjectPtr<UBehaviorTree> BehaviorTree() const;

	TObjectPtr<AActor> FollowedTarget() const;

protected:
	void OnDeath();

	UPROPERTY( EditDefaultsOnly, Category = "Settings|AI" )
	TSubclassOf<AAIController> UnitAIControllerClass_;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|AI" )
	TObjectPtr<UBehaviorTree> UnitBehaviorTree_;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	FEntityStats Stats_;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	TObjectPtr<AActor> FollowedTarget_;

	UPROPERTY()
	TObjectPtr<UCapsuleComponent> CollisionComponent_;

	UPROPERTY()
	TObjectPtr<UFollowComponent> FollowComponent_;

	UPROPERTY()
	TObjectPtr<UAttackComponent> AttackComponent_;
};
