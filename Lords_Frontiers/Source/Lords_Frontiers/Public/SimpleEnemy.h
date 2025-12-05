#pragma once

#include "CoreMinimal.h"
#include "Unit.h"
#include "Components/CapsuleComponent.h"
#include "SimpleEnemy.generated.h"

class UStaticMeshComponent;
class UBehaviorTree;
class AAIController;

/**
 * Minimal spawnable enemy derived from AUnit.
 * Create a Blueprint child (BP_EnemyBasic) to set mesh/behavior and use it for spawning.	chatgpt generated 
 */
UCLASS(Blueprintable)
class LORDS_FRONTIERS_API ASimpleEnemy : public AUnit
{
	GENERATED_BODY()

public:
	ASimpleEnemy();

protected:
	virtual void BeginPlay() override;

public:
	/** Visual mesh so the enemy is visible when spawned. Set a mesh in the Blueprint child. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	/** Maximum health (editable in Blueprint) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "1"))
	int32 MaxHealth = 100;

	/** Current health (replication omitted here; add Replication if needed) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 Health = 100;

	/** Optional: simple attack damage value (not used automatically) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 AttackDamage = 10;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

protected:
	/** Handle death logic (VFX, destroy, etc.) */
	UFUNCTION()
	virtual void HandleDeath();

public:
	/** Implement IAttackable::TakeDamage - called when this unit is attacked */
	virtual void TakeDamage(float DamageAmount) override;
};