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

FEntityStats::FEntityStats(int InMaxHealth, int InAttackDamage, float InAttackRange, float InMoveSpeed)
	: MaxHealth(FMath::Max(1, InMaxHealth))
	, Health(MaxHealth)
	, AttackDamage(InAttackDamage)
	, AttackRange(InAttackRange)
	, MoveSpeed(InMoveSpeed)
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
void FEntityStats::SetMaxHealth(int InMaxHealth)
{
	const int NewMax = FMath::Max(1, InMaxHealth);

	const float Ratio = (MaxHealth > 0) ? (static_cast<float>(Health) / static_cast<float>(MaxHealth)) : 1.0f; //percentage
	MaxHealth = NewMax;

	const int NewHealth = FMath::Clamp(FMath::RoundToInt(Ratio * static_cast<float>(MaxHealth)), 0, MaxHealth);
	Health = NewHealth;
}

void FEntityStats::SetHealth(int InHealth)
{
	Health = FMath::Clamp(InHealth, 0, MaxHealth); //Clamp(X, Min, Max) if X < Min, ret Min; if X > Max, ret Max; 
}

void FEntityStats::SetAttackDamage(int InAttackDamage)
{
	AttackDamage = InAttackDamage;
}

void FEntityStats::SetAttackRange(float InAttackRange)
{
	AttackRange = InAttackRange;
}

void FEntityStats::SetMoveSpeed(float InMoveSpeed)
{
	MoveSpeed = InMoveSpeed;
}


int FEntityStats::ApplyDamage(int Damage)
{
	if (Damage <= 0 || Health <= 0)
	{
		return 0;
	}

	const int PrevHealth = Health;
	Health = FMath::Clamp(Health - Damage, 0, MaxHealth);
	return PrevHealth - Health;
}

void FEntityStats::Heal(int Amount)
{
	if (Amount <= 0 || Health >= MaxHealth)
	{
		return;
	}
	Health = FMath::Clamp(Health + Amount, 0, MaxHealth);
}

void FEntityStats::ModifyAttackDamage(int Delta)
{
	AttackDamage += Delta;
}


bool FEntityStats::IsAlive() const
{
	return Health > 0;
}

bool FEntityStats::IsAtFullHealth() const
{
	return Health >= MaxHealth;
}

