// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "Attacker.h"
#include "ControlledByTree.h"
#include "Entity.h"
#include "EntityStats.h"

#include "Components/Attack/AttackComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "Unit.generated.h"

class AUnitAIManager;
class UPath;
class UCapsuleComponent;
class UBehaviorTree;
class UFollowComponent;
class UNiagaraSystem;
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

	virtual void TakeDamage( int damage ) override;

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
	void SetFollowedTarget( AActor* newTarget );

	const TObjectPtr<UPath>& Path() const;
	void SetPath( UPath* path );

	TObjectPtr<USceneComponent> VisualMesh();

	virtual UNiagaraSystem* GetHitVFX() const override;

protected:
	void OnDeath();

	void SpawnDeathVFX();

	void FinalizeDestroy();

	void ResolveVFXDefaults();

	void FollowNextPathTarget();

	bool IsCloseToTarget() const;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|AI" )
	TSubclassOf<AAIController> UnitAIControllerClass_;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|AI" )
	TObjectPtr<UBehaviorTree> UnitBehaviorTree_;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> DeathVFX_;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> HitVFX_;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|VFX", meta = ( Units = "s" ) )
	float DeathDestroyDelay_ = -1.0f;

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
	TWeakObjectPtr<AUnitAIManager> UnitAIManager_;

	UPROPERTY()
	TObjectPtr<UPath> Path_;

	int PathPointIndex_ = -1;

	// Wobble
	UPROPERTY( EditAnywhere, Category = "Settings|Visuals" )
	float SwaySpeed_ = 15.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|Visuals" )
	float SwayAmplitude_ = 10.0f;

	UPROPERTY()
	TObjectPtr<USceneComponent> VisualMesh_;

	FTimerHandle DeathTimerHandle_;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> ResolvedDeathVFX_;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> ResolvedHitVFX_;

	float ResolvedDeathDestroyDelay_ = 1.0f;
};
