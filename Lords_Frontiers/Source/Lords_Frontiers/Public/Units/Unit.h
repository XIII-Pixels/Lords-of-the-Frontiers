// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "Attacker.h"
#include "ControlledByTree.h"
#include "Entity.h"
#include "EntityStats.h"
#include "Components/Attack/AttackComponent.h"
#include "Components/EnemyAggroComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "Unit.generated.h"

class APathPointsManager;
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

	virtual void Tick( float deltaSeconds ) override;

	void StartFollowing();

	void StopFollowing();

	virtual void Attack( TObjectPtr<AActor> hitActor ) override;

	virtual void TakeDamage( float damage ) override;

	void AdvancePathPointIndex();

	void SetPathPointIndex( int pathPointIndex );

	void FollowPath();

	void ChangeStats( FEnemyBuff* buff );

	// Getters and setters

	virtual FEntityStats& Stats() override;

	virtual ETeam Team() override;

	virtual TObjectPtr<AActor> EnemyInSight() const override;

	virtual TObjectPtr<UBehaviorTree> BehaviorTree() const override;

	TWeakObjectPtr<AActor> FollowedTarget() const;
	void SetFollowedTarget( TObjectPtr<AActor> followedTarget );

	const TObjectPtr<UPath>& Path() const;
	void SetPath( TObjectPtr<UPath> path );

	void SetPathPointsManager( TWeakObjectPtr<APathPointsManager> pathPointsManager );

	void SetFollowedTarget( AActor* newTarget );

	TObjectPtr<USceneComponent> VisualMesh();

protected:
	void OnDeath();

	void FollowNextPathTarget();

	bool IsCloseToTarget() const;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|AI" )
	TSubclassOf<AAIController> UnitAIControllerClass_;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|AI" )
	TObjectPtr<UBehaviorTree> UnitBehaviorTree_;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	FEntityStats Stats_;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	TWeakObjectPtr<AActor> FollowedTarget_;

	UPROPERTY()
	TObjectPtr<UCapsuleComponent> CollisionComponent_;

	UPROPERTY()
	TObjectPtr<UFollowComponent> FollowComponent_;

	UPROPERTY()
	TObjectPtr<UAttackComponent> AttackComponent_;

	UPROPERTY()
	TWeakObjectPtr<APathPointsManager> PathPointsManager_;

	UPROPERTY()
	TObjectPtr<UPath> Path_;

	UPROPERTY()
	TObjectPtr<UEnemyAggroComponent> AggroComponent_;

	UPROPERTY()
	TObjectPtr<USceneComponent> VisualMesh_;

	int PathPointIndex_ = -1;
};
