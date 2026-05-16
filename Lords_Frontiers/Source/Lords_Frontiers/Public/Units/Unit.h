// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "Attacker.h"
#include "ControlledByTree.h"
#include "Entity.h"
#include "EntityStats.h"

#include "Animation/AnimationConfig.h"
#include "Components/Attack/AttackComponent.h"
#include "Components/Attack/UnitAttackRangedComponent.h"
#include "Components/EnemyAggressionComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "Unit.generated.h"

class USpawnAbilityComponent;
class ABuilding;
class AUnitAIManager;
class UPath;
class UCapsuleComponent;
class UBehaviorTree;
class UFollowComponent;
class UNiagaraSystem;
class UHealthBarConfigDataAsset;
struct FEnemyBuff;

USTRUCT(BlueprintType)
struct FUnitAudioTags
{
	GENERATED_BODY()

	UPROPERTY( EditDefaultsOnly )
	FGameplayTag Selected;

	UPROPERTY( EditDefaultsOnly )
	FGameplayTag Spawn;

	UPROPERTY( EditDefaultsOnly )
	FGameplayTag Death;

	UPROPERTY( EditDefaultsOnly )
	FGameplayTag Attack;

	UPROPERTY( EditDefaultsOnly )
	FGameplayTag TakeDamage;

	UPROPERTY( EditDefaultsOnly )
	FGameplayTag SpawnAbility;
};

/** (Gregory-hub)
 * Base class for all units in a game (implement units in blueprints)
 * Can move, attack and be attacked
 * Should be controlled by AI controller */
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API AUnit : public APawn,
                                  public IEntity,
                                  public IAttacker,
                                  public IControlledByTree,
                                  public IAudioEventSource
{
	GENERATED_BODY()

public:
	AUnit();

	virtual void OnConstruction( const FTransform& transform ) override;

	virtual void BeginPlay() override;
	virtual void EndPlay( const EEndPlayReason::Type endPlayReason ) override;

	virtual void PostInitProperties() override;

	void StartFollowing() const;
	void StopFollowing() const;

	void EnableMovement() const;
	void DisableMovement() const;

	void EnableAttack();
	void DisableAttack();

	virtual void Attack( TObjectPtr<AActor> hitActor ) override;

	virtual void TakeDamage( int damage, AActor* instigator = nullptr ) override;

	void ChangeStats( const FEnemyBuff* buff );

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

	virtual ETeam Team() override
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

	USkeletalMeshComponent* SkeletalMeshComponent()
	{
		return SkeletalMeshComponent_;
	}

	virtual UNiagaraSystem* GetHitVFX() const override;

	UHealthBarConfigDataAsset* HealthBarConfig() const
	{
		return HealthBarConfig_;
	}

	UFUNCTION( BlueprintPure, Category = "Settings|Boss" )
	bool IsBoss() const
	{
		return bIsBoss_;
	}

	UFUNCTION( BlueprintPure, Category = "Settings|Boss" )
	bool IsBossForStats() const
	{
		return bIsBossForStats_;
	}

	virtual FOnAudioEvent& GetOnAudioEvent() override
	{
		return OnAudioEvent_;
	}

	const FUnitAudioTags& AudioTags() const
	{
		return AudioTags_;
	}

	bool OnlyAttackTargetBuilding() const
	{
		if ( const auto* attackComponent = Cast<UUnitAttackRangedComponent>( AttackComponent_ ) )
		{
			return attackComponent->OnlyAttackTargetBuilding();
		}
		return false;
	}

	bool PlayAnimationIdle();
	bool PlayAnimationAttack();
	bool PlayAnimationSpawnAbility();

protected:
	virtual void Tick( float deltaSeconds ) override;

	void OnDeath();

	void SpawnSpawnVFX();

	void SpawnDeathVFX();

	void ResolveVFXDefaults();

	void Animate();

	bool PlayAnimation( const FAnimationConfig& animation ) const;

	void ResolveAudioTags();

	UPROPERTY( EditDefaultsOnly, Category = "Settings|AI" )
	TSubclassOf<AAIController> UnitAIControllerClass_;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|AI" )
	TObjectPtr<UBehaviorTree> UnitBehaviorTree_;

	UPROPERTY( EditAnywhere, Category = "Settings" )
	FEntityStats Stats_;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|HealthBar" )
	TObjectPtr<UHealthBarConfigDataAsset> HealthBarConfig_;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Boss", meta = ( DisplayName = "Босс (полоса HP)", ToolTip = "Включает оформление полосы здоровья как у босса. На подсчёт очков и статистику не влияет." ) )
	bool bIsBoss_ = false;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Boss", meta = ( DisplayName = "Босс (статистика и счёт)", ToolTip = "Если включено, юнит учитывается как босс в статистике матча и в подсчёте очков. От полосы HP не зависит." ) )
	bool bIsBossForStats_ = false;

	FDelegateHandle HealthBarSubscription_;

	UPROPERTY( VisibleInstanceOnly, Category = "Settings" )
	TWeakObjectPtr<const AActor> FollowedTarget_;

	UPROPERTY( VisibleInstanceOnly, Category = "Settings" )
	TWeakObjectPtr<AActor> AttackTarget_;

	UPROPERTY( VisibleInstanceOnly, Category = "Settings" )
	TWeakObjectPtr<const ABuilding> TargetBuilding_;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Animation" )
	FAnimationConfig IdleAnimation_;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Animation" )
	FAnimationConfig AttackAnimation_;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Animation" )
	FAnimationConfig SpawnAbilityAnimation_;

	UPROPERTY(
	    EditDefaultsOnly, Category = "Settings",
	    meta =
	        ( ClampMin = 0.0f, Units = "s",
	          ToolTip = "Adjust this so unit attack hit moment aligns with animation attack moment" )
	)
	float AttackPreHitDelay_ = 0.0f;

	UPROPERTY(
	    EditDefaultsOnly, Category = "Settings",
	    meta =
	        ( ClampMin = 0.0f, Units = "s",
	          ToolTip = "Adjust this so spawn start moment aligns with animation spawn moment" )
	)
	float PreSpawnAbilityDelay_ = 0.0f;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Audio" )
	FUnitAudioTags AudioTags_;
  
	UPROPERTY()
	TObjectPtr<UCapsuleComponent> CollisionComponent_;

	UPROPERTY()
	TObjectPtr<UFollowComponent> FollowComponent_;

	UPROPERTY( EditDefaultsOnly )
	TObjectPtr<UAttackComponent> AttackComponent_;

	UPROPERTY()
	TWeakObjectPtr<USpawnAbilityComponent> SpawnAbilityComponent_;

	UPROPERTY( EditDefaultsOnly )
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent_;

	UPROPERTY()
	TWeakObjectPtr<AUnitAIManager> UnitAIManager_;

	FTimerHandle AttackTimerHandle_;
	FTimerHandle AnimationTickTimerHandle_;
	FTimerHandle DeathTimerHandle_;
	FTimerHandle DeathVFXTimerHandle_;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> ResolvedSpawnVFX_;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> ResolvedDeathVFX_;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> ResolvedHitVFX_;

	float ResolvedDeathVFXDelay_ = 0.0f;

	FOnAudioEvent OnAudioEvent_;
  
	UPROPERTY( VisibleAnywhere, Category = "Settings")
	bool bCanAttack = true;

	bool bIdleIsAnimated_ = false;

	bool bPlayingIdleAnimation = false;
};
