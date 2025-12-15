#pragma once

#include "CoreMinimal.h"
#include "EntityStats.generated.h"

// (Artyom)
// entity stats struct (health, damage, damage radius, speed)
USTRUCT( BlueprintType )
struct FEntityStats
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" )
	int MaxHealth = 100;

	UPROPERTY( VisibleInstanceOnly, BlueprintReadWrite, Category = "Stats" )
	int Health = MaxHealth;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" )
	int AttackDamage = 10;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" )
	float AttackRange = 200.0f;

public:
	FEntityStats() = default;

	explicit FEntityStats(int maxHealth, int attackDamage, float attackRange, float moveSpeed);

	// getters
	int GetMaxHealth() const;

	int GetHealth() const;

	int GetAttackDamage() const;

	float GetAttackRange() const;

	// setters
	void SetMaxHealth(int maxHealth); // min = 1

	void SetHealth(int health);

	void SetAttackDamage(int attackDamage);

	void SetAttackRange(float attackRange);

	// functionality
	// return applied damage
	int ApplyDamage(int damage);

	void Heal(int amount);

	// damage modificator (if needed)
	void ModifyAttackDamage(int delta);

	// utils
	bool IsAlive() const;

	bool IsAtFullHealth() const;
};
