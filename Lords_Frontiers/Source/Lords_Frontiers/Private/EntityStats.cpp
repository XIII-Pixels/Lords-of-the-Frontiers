#include "Lords_Frontiers/Public/EntityStats.h"
#include "Math/UnrealMathUtility.h" // FMath

FEntityStats::FEntityStats ( int maxHealth, int attackDamage, float attackRange, float moveSpeed )
	: MaxHealth_ ( FMath::Max ( 1, maxHealth ) )
	, Health_ ( MaxHealth_ )
	, AttackDamage_ ( attackDamage )
	, AttackRange_ ( attackRange )
{
}

// getters

int FEntityStats::GetMaxHealth () const
{
	return MaxHealth_;
}

int FEntityStats::GetHealth () const
{
	return Health_;
}

int FEntityStats::GetAttackDamage () const
{
	return AttackDamage_;
}

float FEntityStats::GetAttackCooldown() const
{
	return AttackCooldown_;
}

void FEntityStats::SetAttackCooldown(float attackCooldown)
{
	AttackCooldown_ = attackCooldown;
}

float FEntityStats::GetAttackRange () const
{
	return AttackRange_;
}

// setters
// (Artyom)
// entity keep hp percentage at the same lvl;  before (50/100hp); SetMaxHealth(150); after (75/150hp)
void FEntityStats::SetMaxHealth ( int maxHealth )
{
	const int newMax = FMath::Max ( 1, maxHealth );

	const float ratio = ( MaxHealth_ > 0 ) ? ( static_cast<float> ( Health_ ) / static_cast<float> ( MaxHealth_ ) ) : 1.0f; //percentage
	MaxHealth_ = newMax;

	const int newHealth = FMath::Clamp ( FMath::RoundToInt ( ratio * static_cast<float> ( MaxHealth_ ) ), 0, MaxHealth_ );
	Health_ = newHealth;
}

void FEntityStats::SetHealth ( int health )
{
	Health_ = FMath::Clamp ( health, 0, MaxHealth_ ); //Clamp(X, Min, Max) if X < Min, ret Min; if X > Max, ret Max; 
}

void FEntityStats::SetAttackDamage ( int attackDamage )
{
	AttackDamage_ = attackDamage;
}

void FEntityStats::SetAttackRange ( float attackRange )
{
	AttackRange_ = attackRange;
}

int FEntityStats::ApplyDamage ( int damage )
{
	if ( damage <= 0 || Health_ <= 0 )
	{
		return 0;
	}

	const int prevHealth = Health_;
	Health_ = FMath::Clamp ( Health_ - damage, 0, MaxHealth_ );
	return prevHealth - Health_;
}

void FEntityStats::Heal ( int amount )
{
	if ( amount <= 0 || Health_ >= MaxHealth_ )
	{
		return;
	}
	Health_ = FMath::Clamp ( Health_ + amount, 0, MaxHealth_ );
}

bool FEntityStats::OnCooldown() const
{
	return (FDateTime::Now() - LastAttackTime_).GetTotalSeconds() < AttackCooldown_;
}

void FEntityStats::StartCooldown()
{
	LastAttackTime_ = FDateTime::Now();
}

void FEntityStats::ModifyAttackDamage ( int delta )
{
	AttackDamage_ += delta;
}

bool FEntityStats::IsAlive () const
{
	return Health_ > 0;
}

bool FEntityStats::IsAtFullHealth () const
{
	return Health_ >= MaxHealth_;
}
