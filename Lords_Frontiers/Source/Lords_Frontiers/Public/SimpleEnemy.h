#pragma once

#include "Lords_Frontiers/Public/Units/Unit.h"

#include "Components/CapsuleComponent.h"
#include "CoreMinimal.h"

#include "SimpleEnemy.generated.h"

class UStaticMeshComponent;
class UBehaviorTree;
class AAIController;

/**
 * Minimal spawnable enemy derived from AUnit.
 * Create a Blueprint child (BP_EnemyBasic) to set mesh/behavior and use it for
 * spawning.	chatgpt generated this wont be included in actual build
 */
UCLASS( Blueprintable )
class LORDS_FRONTIERS_API ASimpleEnemy : public AUnit
{
	GENERATED_BODY()

public:
	ASimpleEnemy();

	/** Visual mesh so the enemy is visible when spawned. Set a mesh in the
	 * Blueprint child. */
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Components" )
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	/** Maximum health (editable in Blueprint) */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = ( ClampMin = "1" ) )
	float MaxHealth;

	/** Current health (replication omitted here; add Replication if needed) */
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Combat" )
	float Health;

	/** Optional: simple attack damage value (not used automatically) */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Combat" )
	int32 AttackDamage = 10;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Components" )
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	// Editable defaults so designer can tweak per BP
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Collision" )
	float DefaultCapsuleRadius = 34.f;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Collision" )
	float DefaultCapsuleHalfHeight = 88.f;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Visual" )
	FVector DefaultMeshRelativeLocation = FVector( 0.f, 0.f, -88.f );

protected:
	/** Handle death logic (VFX, destroy, etc.) */
	UFUNCTION()
	virtual void HandleDeath();

	virtual void BeginPlay() override;

public:
	/** Implement IAttackable::TakeDamage - called when this unit is attacked */
	virtual void TakeDamage( float DamageAmount ) override;
};