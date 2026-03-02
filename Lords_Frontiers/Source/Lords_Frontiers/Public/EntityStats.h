#pragma once

#include "CoreMinimal.h"

#include "EntityStats.generated.h"

UENUM( BlueprintType )
enum class ETeam : uint8
{
	Dog,
	Cat
};

UENUM( BlueprintType )
enum class EStatsType : uint8
{
	MaxHealth,
	AttackRange,
	AttackDamage,
	AttackCooldown,
	MaxSpeed,
	SplashRadius,
	BurstCount,
	BurstDelay
};

UENUM( BlueprintType )
enum class EBurstTargetMode : uint8
{
	SameTarget UMETA( DisplayName = "Same Target" ),
	Neighbors UMETA( DisplayName = "Neighboring Enemies" )
};

// (Artyom)
// entity stats struct (health, damage, damage radius, speed)
USTRUCT( BlueprintType )
struct FEntityStats
{
	GENERATED_BODY()

	FEntityStats() = default;

	explicit FEntityStats( int maxHealth, int attackDamage, float attackRange );

	// getters
	int MaxHealth() const;

	int Health() const;

	float AttackRange() const;

	int AttackDamage() const;

	float AttackCooldown() const;

	float MaxSpeed() const;

	ETeam Team() const;

	float SplashRadius() const;

	int32 BurstCount() const;

	float BurstDelay() const;

	EBurstTargetMode BurstTargetMode() const;
	// setters
	void SetMaxHealth( int maxHealth ); // min = 1

	void SetHealth( int health );

	void SetAttackRange( float attackRange );

	void SetAttackDamage( int attackDamage );

	void SetAttackCooldown( float attackCooldown );

	void SetMaxSpeed( float maxSpeed );

	void SetTeam( ETeam team );

	void SetSplashRadius( float splashRadius );

	void SetBurstCount( int32 burstCount );

	void SetBurstDelay( float burstDelay );

	void SetBurstTargetMode( EBurstTargetMode burstTargetMode );

	void AddSplashRadius( float splashRadius );

	void AddStat( EStatsType statType, float value );

	void AddMaxHealth( int maxHealth );

	void AddAttackRange( float attackRange );

	void AddAttackDamage( int attackDamage );

	void AddAttackCooldown( float attackCooldown );

	void AddMaxSpeed( float maxSpeed );

	void AddBurstCount( int32 burstCount );

	void AddBurstDelay( float burstDelay );

	// returns applied damage
	int ApplyDamage( int damage );

	void Heal( int amount );

	bool OnCooldown() const;

	void StartCooldown();

	// damage modificator (if needed)
	void ModifyAttackDamage( int delta );

	// utils
	bool IsAlive() const;

	bool IsAtFullHealth() const;

private:
	UPROPERTY( EditAnywhere, Category = "Settings|Stats" )
	int MaxHealth_ = 100;

	UPROPERTY( VisibleInstanceOnly, Category = "Settings|Stats" )
	int Health_;

	UPROPERTY( EditAnywhere, Category = "Settings|Stats|Attack" )
	float AttackRange_ = 200.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|Stats|Attack" )
	int AttackDamage_ = 10;

	UPROPERTY( EditAnywhere, Category = "Settings|Stats|Attack" )
	float AttackCooldown_ = 1.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|Stats" )
	float MaxSpeed_ = 300.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|Stats|Attack" )
	float SplashRadius_ = 0.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|Stats|Attack", meta = ( ClampMin = "1", ClampMax = "10" ) )
	int32 BurstCount_ = 1;

	UPROPERTY( EditAnywhere, Category = "Settings|Stats|Attack", meta = ( ClampMin = "0.05" ) )
	float BurstDelay_ = 0.15f;

	UPROPERTY( EditAnywhere, Category = "Settings|Stats|Attack" )
	EBurstTargetMode BurstTargetMode_ = EBurstTargetMode::SameTarget;

	UPROPERTY( EditAnywhere, Category = "Settings|Stats" )
	ETeam Team_;

	FDateTime LastAttackTime_;
};
