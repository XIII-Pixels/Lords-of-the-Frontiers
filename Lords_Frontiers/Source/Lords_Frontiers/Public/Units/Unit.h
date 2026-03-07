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

	TWeakObjectPtr<AActor> FollowedTarget() const;

	const TObjectPtr<UPath>& Path() const;

	void SetFollowedTarget( TWeakObjectPtr<AActor> newTarget );

	void ChangeStats( FEnemyBuff* buff );

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

	virtual UNiagaraSystem* GetHitVFX() const override;

protected:
	void OnDeath();

	void SpawnDeathVFX();

	void FinalizeDestroy();

	void ResolveVFXDefaults();

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
