#include "Lords_Frontiers/Public/EntityStats.h"
#include "Math/UnrealMathUtility.h" // FMath

FEntityStats::FEntityStats ( int maxHealth, int attackDamage, float attackRange, float moveSpeed )
	: MaxHealth ( FMath::Max ( 1, maxHealth ) )
	, Health ( MaxHealth )
	, AttackDamage ( attackDamage )
	, AttackRange ( attackRange )
{
}

// getters

int FEntityStats::GetMaxHealth () const
{
	return MaxHealth;
}

int FEntityStats::GetHealth () const
{
	return Health;
}

int FEntityStats::GetAttackDamage () const
{
	return AttackDamage;
}

float FEntityStats::GetAttackRange () const
{
	return AttackRange;
}

// setters
// (Artyom)
// entity keep hp percentage at the same lvl;  before (50/100hp); SetMaxHealth(150); after (75/150hp)
void FEntityStats::SetMaxHealth ( int maxHealth )
{
	const int newMax = FMath::Max ( 1, maxHealth );

	const float ratio = ( MaxHealth > 0 ) ? ( static_cast<float> ( Health ) / static_cast<float> ( MaxHealth ) ) : 1.0f; //percentage
	MaxHealth = newMax;

	const int newHealth = FMath::Clamp ( FMath::RoundToInt ( ratio * static_cast<float> ( MaxHealth ) ), 0, MaxHealth );
	Health = newHealth;
}

void FEntityStats::SetHealth ( int health )
{
	Health = FMath::Clamp ( health, 0, MaxHealth ); //Clamp(X, Min, Max) if X < Min, ret Min; if X > Max, ret Max; 
}

void FEntityStats::SetAttackDamage ( int attackDamage )
{
	AttackDamage = attackDamage;
}

void FEntityStats::SetAttackRange ( float attackRange )
{
	AttackRange = attackRange;
}

int FEntityStats::ApplyDamage ( int damage )
{
	if ( damage <= 0 || Health <= 0 )
	{
		return 0;
	}

	const int prevHealth = Health;
	Health = FMath::Clamp ( Health - damage, 0, MaxHealth );
	return prevHealth - Health;
}

void FEntityStats::Heal ( int amount )
{
	if ( amount <= 0 || Health >= MaxHealth )
	{
		return;
	}
	Health = FMath::Clamp ( Health + amount, 0, MaxHealth );
}

void FEntityStats::ModifyAttackDamage ( int delta )
{
	AttackDamage += delta;
}


bool FEntityStats::IsAlive () const
{
	return Health > 0;
}

bool FEntityStats::IsAtFullHealth () const
{
	return Health >= MaxHealth;
}

