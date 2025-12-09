#pragma once

#include "CoreMinimal.h"
#include "EntityStats.generated.h"

// (Artyom)
// entity stats struct (health, damage, damage radius, speed)
USTRUCT ( BlueprintType )
struct FEntityStats
{
	GENERATED_BODY ()

public:
	UPROPERTY ( EditAnywhere, BlueprintReadWrite, Category = "Stats" )
	int MaxHealth;

	UPROPERTY ( EditAnywhere, BlueprintReadWrite, Category = "Stats" )
	int Health;

	UPROPERTY ( EditAnywhere, BlueprintReadWrite, Category = "Stats" )
	int AttackDamage;

	UPROPERTY ( EditAnywhere, BlueprintReadWrite, Category = "Stats" )
	float AttackRange;

	UPROPERTY ( EditAnywhere, BlueprintReadWrite, Category = "Stats" )
	float MoveSpeed;

public:
	FEntityStats ();
	explicit FEntityStats ( int inMaxHealth, int inAttackDamage, float inAttackRange, float inMoveSpeed );

	// getters
	int GetMaxHealth () const;
	int GetHealth () const;
	int GetAttackDamage () const;
	float GetAttackRange () const;
	float GetMoveSpeed () const;

	// setters
	void SetMaxHealth ( int inMaxHealth ); // min = 1

	void SetHealth ( int inHealth );

	void SetAttackDamage ( int inAttackDamage );
	void SetAttackRange ( float inAttackRange );
	void SetMoveSpeed ( float inMoveSpeed );

	// functionality
	// return applied damage
	int ApplyDamage ( int damage );

	void Heal ( int amount );

	// damage modificator (if needed)
	void ModifyAttackDamage ( int delta );

	// utils
	bool IsAlive () const;
	bool IsAtFullHealth () const;
};
