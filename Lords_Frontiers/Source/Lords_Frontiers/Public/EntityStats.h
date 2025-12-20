#pragma once

#include "CoreMinimal.h"
#include "EntityStats.generated.h"


UENUM( BlueprintType )
enum class ETeam : uint8
{
	Dog,
	Cat
};


// (Artyom)
// entity stats struct (health, damage, damage radius, speed)
USTRUCT( BlueprintType )
struct FEntityStats
{
	GENERATED_BODY()

public:
	FEntityStats() = default;

	explicit FEntityStats(int maxHealth, int attackDamage, float attackRange, float moveSpeed);

	// getters
	int MaxHealth() const;

	int Health() const;

	float AttackRange() const;

	int AttackDamage() const;

	float AttackCooldown() const;

	ETeam Team() const;

	// setters
	void SetMaxHealth(int maxHealth); // min = 1

	void SetHealth(int health);

	void SetAttackRange(float attackRange);

	void SetAttackDamage(int attackDamage);
	
	void SetAttackCooldown(float attackCooldown);
	
	void SetTeam(ETeam team);

	// functionality
	
	// returns applied damage
	int ApplyDamage(int damage);

	void Heal(int amount);

	bool OnCooldown() const;

	void StartCooldown();

	// damage modificator (if needed)
	void ModifyAttackDamage(int delta);

	// utils
	bool IsAlive() const;

	bool IsAtFullHealth() const;

private:
	UPROPERTY( EditAnywhere, Category = "Stats" )
	int MaxHealth_ = 100;

	UPROPERTY( VisibleInstanceOnly, Category = "Stats" )
	int Health_ = MaxHealth_;

	UPROPERTY( EditAnywhere, Category = "Stats" )
	float AttackRange_ = 200.0f;

	UPROPERTY( EditAnywhere, Category = "Stats" )
	int AttackDamage_ = 10;

	UPROPERTY( EditAnywhere, Category = "Stats" )
	float AttackCooldown_ = 1.0f;

	UPROPERTY( EditAnywhere, Category = "Stats" )
	ETeam Team_;

	FDateTime LastAttackTime_;
};
