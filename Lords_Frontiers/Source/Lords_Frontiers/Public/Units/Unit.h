// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "Attacker.h"
#include "ControlledByTree.h"
#include "Entity.h"
#include "EntityStats.h"
#include "Components/Attack/AttackComponent.h"
#include "Components/EnemyAggressionComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "Unit.generated.h"

class ABuilding;
class AUnitAIManager;
class UPath;
class UCapsuleComponent;
class UBehaviorTree;
class UFollowComponent;
struct FEnemyBuff;

/** (Gregory-hub)
 * Base class for all units in a game (implement units in blueprints)
 * Can move, attack and be attacked
 * Should be controlled by AI controller */
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API AUnit : public APawn, public IEntity, public IAttacker, public IControlledByTree
{
	GENERATED_BODY()

public:
	AUnit();

	virtual void OnConstruction( const FTransform& transform ) override;

	virtual void BeginPlay() override;

	void StartFollowing() const;

	void StopFollowing() const;

	virtual void Attack( TObjectPtr<AActor> hitActor ) override;

	virtual void TakeDamage( int damage ) override;

	void ChangeStats( FEnemyBuff* buff );

	virtual FEntityStats& Stats() override
	{
		return Stats_;
	}

	virtual const FEntityStats& Stats() const override
	{
		return Stats_;
	}

	virtual ETeam Team() const override
	{
		return Stats_.Team();
	}

	virtual TObjectPtr<UBehaviorTree> BehaviorTree() const override
	{
		return UnitBehaviorTree_;
	}

	// Target that unit moves to
	TWeakObjectPtr<const AActor> FollowedTarget() const
	{
		return FollowedTarget_;
	}

	void SetFollowedTarget( TWeakObjectPtr<const AActor> newTarget )
	{
		FollowedTarget_ = newTarget;
	}

	// Target that unit might attack
	virtual TWeakObjectPtr<AActor> AttackTarget() const override
	{
		return AttackTarget_;
	}

	virtual void SetAttackTarget( TWeakObjectPtr<AActor> newTarget ) override
	{
		AttackTarget_ = newTarget;
	}

	// Path destination
	TWeakObjectPtr<const ABuilding> TargetBuilding() const
	{
		return TargetBuilding_;
	}

	void SetTargetBuilding( TWeakObjectPtr<const ABuilding> newTarget )
	{
		TargetBuilding_ = newTarget;
	}

	UPath* Path() const
	{
		if ( const UEnemyAggressionComponent* aggression = GetComponentByClass<UEnemyAggressionComponent>() )
		{
			return aggression->Path();
		}
		return nullptr;
	}

	TObjectPtr<USceneComponent> VisualMesh()
	{
		return VisualMesh_;
	}

protected:
	void OnDeath();

	UPROPERTY( EditDefaultsOnly, Category = "Settings|AI" )
	TSubclassOf<AAIController> UnitAIControllerClass_;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|AI" )
	TObjectPtr<UBehaviorTree> UnitBehaviorTree_;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	FEntityStats Stats_;

	UPROPERTY( VisibleInstanceOnly, Category = "Settings" )
	TWeakObjectPtr<const AActor> FollowedTarget_;

	UPROPERTY( VisibleInstanceOnly, Category = "Settings" )
	TWeakObjectPtr<AActor> AttackTarget_;

	UPROPERTY( VisibleInstanceOnly, Category = "Settings" )
	TWeakObjectPtr<const ABuilding> TargetBuilding_;

	UPROPERTY()
	TObjectPtr<UCapsuleComponent> CollisionComponent_;

	UPROPERTY()
	TObjectPtr<UFollowComponent> FollowComponent_;

	UPROPERTY()
	TObjectPtr<UAttackComponent> AttackComponent_;

	UPROPERTY()
	TWeakObjectPtr<AUnitAIManager> UnitAIManager_;

	UPROPERTY()
	TObjectPtr<USceneComponent> VisualMesh_;
};
