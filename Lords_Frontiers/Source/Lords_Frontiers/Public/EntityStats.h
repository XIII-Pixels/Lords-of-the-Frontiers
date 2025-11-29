#pragma once

#include "CoreMinimal.h"
#include "EntityStats.generated.h"

// (Artyom)
// entity stats struct (health, damage, damage radius, speed)
USTRUCT(BlueprintType)
struct FEntityStats
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int AttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MoveSpeed;

public:
	FEntityStats();
	explicit FEntityStats(int InMaxHealth, int InAttackDamage, float InAttackRange, float InMoveSpeed);

	// getters
	int GetMaxHealth() const;
	int GetHealth() const;
	int GetAttackDamage() const;
	float GetAttackRange() const;
	float GetMoveSpeed() const;

	// setters
	void SetMaxHealth(int InMaxHealth); // min = 1

	void SetHealth(int InHealth);

	void SetAttackDamage(int InAttackDamage);
	void SetAttackRange(float InAttackRange);
	void SetMoveSpeed(float InMoveSpeed);

	// functionality
	// return applied damage
	int ApplyDamage(int Damage);

	void Heal(int Amount);

	// Изменить (прибавить) значение урона (полезно для баффов/дебаффов)
	void ModifyAttackDamage(int Delta);

	// --- Состояния/утилиты ---
	bool IsAlive() const;
	bool IsAtFullHealth() const;
};
