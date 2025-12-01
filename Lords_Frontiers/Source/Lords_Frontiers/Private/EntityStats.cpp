#include "Lords_Frontiers/Public/EntityStats.h"
#include "Math/UnrealMathUtility.h" // FMath

FEntityStats::FEntityStats()
	: MaxHealth(100)
	, Health(100)
	, AttackDamage(10)
	, AttackRange(200.0f)
	, MoveSpeed(600.0f)
{
}

FEntityStats::FEntityStats(int inMaxHealth, int inAttackDamage, float inAttackRange, float inMoveSpeed)
	: MaxHealth(FMath::Max(1, inMaxHealth))
	, Health(MaxHealth)
	, AttackDamage(inAttackDamage)
	, AttackRange(inAttackRange)
	, MoveSpeed(inMoveSpeed)
{
}

// getters

int FEntityStats::GetMaxHealth() const
{
	return MaxHealth;
}

int FEntityStats::GetHealth() const
{
	return Health;
}

int FEntityStats::GetAttackDamage() const
{
	return AttackDamage;
}

float FEntityStats::GetAttackRange() const
{
	return AttackRange;
}

float FEntityStats::GetMoveSpeed() const
{
	return MoveSpeed;
}


// setters
// (Artyom)
// entity keep hp percentage at the same lvl;  before (50/100hp); SetMaxHealth(150); after (75/150hp)
void FEntityStats::SetMaxHealth(int inMaxHealth)
{
	const int newMax = FMath::Max(1, inMaxHealth);

	const float Ratio = (MaxHealth > 0) ? (static_cast<float>(Health) / static_cast<float>(MaxHealth)) : 1.0f; //percentage
	MaxHealth = newMax;

	const int newHealth = FMath::Clamp(FMath::RoundToInt(Ratio * static_cast<float>(MaxHealth)), 0, MaxHealth);
	Health = newHealth;
}

void FEntityStats::SetHealth(int inHealth)
{
	Health = FMath::Clamp(inHealth, 0, MaxHealth); //Clamp(X, Min, Max) if X < Min, ret Min; if X > Max, ret Max; 
}

void FEntityStats::SetAttackDamage(int inAttackDamage)
{
	AttackDamage = inAttackDamage;
}

void FEntityStats::SetAttackRange(float inAttackRange)
{
	AttackRange = inAttackRange;
}

void FEntityStats::SetMoveSpeed(float inMoveSpeed)
{
	MoveSpeed = inMoveSpeed;
}


int FEntityStats::ApplyDamage(int damage)
{
	if (damage <= 0 || Health <= 0)
	{
		return 0;
	}

	const int prevHealth = Health;
	Health = FMath::Clamp(Health - damage, 0, MaxHealth);
	return prevHealth - Health;
}

void FEntityStats::Heal(int amount)
{
	if (amount <= 0 || Health >= MaxHealth)
	{
		return;
	}
	Health = FMath::Clamp(Health + amount, 0, MaxHealth);
}

void FEntityStats::ModifyAttackDamage(int delta)
{
	AttackDamage += delta;
}


bool FEntityStats::IsAlive() const
{
	return Health > 0;
}

bool FEntityStats::IsAtFullHealth() const
{
	return Health >= MaxHealth;
}

