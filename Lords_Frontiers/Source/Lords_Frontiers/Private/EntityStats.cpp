#include "Lords_Frontiers/Public/EntityStats.h"

#include "Math/UnrealMathUtility.h" // FMath

FEntityStats::FEntityStats( int maxHealth, int attackDamage, float attackRange )
    : MaxHealth_( FMath::Max( 1, maxHealth ) ), Health_( MaxHealth_ ), AttackRange_( attackRange ),
      AttackDamage_( attackDamage )
{
}

// getters

int FEntityStats::MaxHealth() const
{
	return MaxHealth_;
}

int FEntityStats::Health() const
{
	return Health_;
}

int FEntityStats::AttackDamage() const
{
	return AttackDamage_;
}

float FEntityStats::AttackCooldown() const
{
	return AttackCooldown_;
}

float FEntityStats::MaxSpeed() const
{
	return MaxSpeed_;
}

ETeam FEntityStats::Team() const
{
	return Team_;
}

float FEntityStats::AttackRange() const
{
	return AttackRange_;
}

float FEntityStats::SplashRadius() const
{
	return SplashRadius_;
}

int32 FEntityStats::BurstCount() const
{
	return BurstCount_;
}

float FEntityStats::BurstDelay() const
{
	return BurstDelay_;
}

EBurstTargetMode FEntityStats::BurstTargetMode() const
{
	return BurstTargetMode_;
}

// setters

// (Artyom)
// entity keep hp percentage at the same lvl;  before (50/100hp);
// SetMaxHealth(150); after (75/150hp)
void FEntityStats::SetMaxHealth( int maxHealth )
{
	const int newMax = FMath::Max( 1, maxHealth );

	const float ratio =
	    ( MaxHealth_ > 0 ) ? ( static_cast<float>( Health_ ) / static_cast<float>( MaxHealth_ ) ) : 1.0f; // percentage
	MaxHealth_ = newMax;

	const int newHealth = FMath::Clamp( FMath::RoundToInt( ratio * static_cast<float>( MaxHealth_ ) ), 0, MaxHealth_ );
	Health_ = newHealth;
}

void FEntityStats::SetHealth( int health )
{
	Health_ = FMath::Clamp( health, 0, MaxHealth_ ); // Clamp(X, Min, Max) if X < Min, ret
	                                                 // Min; if X > Max, ret Max;
}

void FEntityStats::SetAttackDamage( int attackDamage )
{
	AttackDamage_ = attackDamage;
}

void FEntityStats::SetAttackRange( float attackRange )
{
	AttackRange_ = attackRange;
}

void FEntityStats::SetTeam( ETeam team )
{
	Team_ = team;
}

void FEntityStats::SetAttackCooldown( float attackCooldown )
{
	AttackCooldown_ = attackCooldown;
}

void FEntityStats::SetMaxSpeed( float maxSpeed )
{
	MaxSpeed_ = FMath::Max( maxSpeed, 0.0f );
}

void FEntityStats::SetSplashRadius( float splashRadius )
{
	SplashRadius_ = FMath::Max( splashRadius, 0.0f );
}

void FEntityStats::SetBurstCount( int32 burstCount )
{
	BurstCount_ = FMath::Max( burstCount, 1 );
}

void FEntityStats::SetBurstDelay( float burstDelay )
{
	BurstDelay_ = FMath::Max( burstDelay, 0.05f );
}

void FEntityStats::SetBurstTargetMode( EBurstTargetMode burstTargetMode )
{
	BurstTargetMode_ = burstTargetMode;
}

void FEntityStats::AddSplashRadius( float splashRadius )
{
	SetSplashRadius( SplashRadius_ + splashRadius );
}

void FEntityStats::AddStat( EStatsType statType, float value )
{
	switch ( statType )
	{
	case EStatsType::MaxHealth:
		AddMaxHealth( FMath::FloorToInt( value ) );
		break;
	case EStatsType::AttackRange:
		AddAttackRange( value );
		break;
	case EStatsType::AttackDamage:
		AddAttackDamage( FMath::FloorToInt( value ) );
		break;
	case EStatsType::AttackCooldown:
		AddAttackCooldown( value );
		break;
	case EStatsType::MaxSpeed:
		break;
	case EStatsType::SplashRadius:
		AddSplashRadius( value );
		break;
	case EStatsType::BurstCount:
		AddBurstCount( FMath::FloorToInt( value ) );
		break;
	case EStatsType::BurstDelay:
		AddBurstDelay( value );
		break;
	default:
		break;
	}
}

void FEntityStats::AddMaxHealth( int maxHealth )
{
	SetMaxHealth( MaxHealth_ + maxHealth );
}

void FEntityStats::AddAttackRange( float attackRange )
{
	SetAttackRange( AttackRange_ + attackRange );
}

void FEntityStats::AddAttackDamage( int attackDamage )
{
	SetAttackDamage( AttackDamage_ + attackDamage );
}

void FEntityStats::AddAttackCooldown( float attackCooldown )
{
	SetAttackCooldown( AttackCooldown_ + attackCooldown );
}

void FEntityStats::AddMaxSpeed( float maxSpeed )
{
	SetMaxSpeed( MaxSpeed_ + maxSpeed );
}

void FEntityStats::AddBurstCount( int32 burstCount )
{
	SetBurstCount( BurstCount_ + burstCount );
}

void FEntityStats::AddBurstDelay( float burstDelay )
{
	SetBurstDelay( BurstDelay_ + burstDelay );
}

int FEntityStats::ApplyDamage( int damage )
{
	if ( damage <= 0 || Health_ <= 0 )
	{
		return 0;
	}

	const int prevHealth = Health_;
	Health_ = FMath::Clamp( Health_ - damage, 0, MaxHealth_ );
	return prevHealth - Health_;
}

void FEntityStats::Heal( int amount )
{
	if ( amount <= 0 || Health_ >= MaxHealth_ )
	{
		return;
	}
	Health_ = FMath::Clamp( Health_ + amount, 0, MaxHealth_ );
}

bool FEntityStats::OnCooldown() const
{
	return ( FDateTime::Now() - LastAttackTime_ ).GetTotalSeconds() < AttackCooldown_;
}

void FEntityStats::StartCooldown()
{
	LastAttackTime_ = FDateTime::Now();
}

void FEntityStats::ModifyAttackDamage( int delta )
{
	AttackDamage_ += delta;
}

bool FEntityStats::IsAlive() const
{
	return Health_ > 0;
}

bool FEntityStats::IsAtFullHealth() const
{
	return Health_ >= MaxHealth_;
}
