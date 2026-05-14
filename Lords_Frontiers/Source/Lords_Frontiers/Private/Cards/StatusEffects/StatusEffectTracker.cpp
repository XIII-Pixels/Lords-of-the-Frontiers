#include "Cards/StatusEffects/StatusEffectTracker.h"

#include "Cards/CardPoolConfig.h"
#include "Cards/CardSubsystem.h"
#include "Cards/StatusEffects/StatusEffectDef.h"
#include "Cards/Visuals/CardVisualSubsystem.h"
#include "Components/FollowComponent.h"
#include "Entity.h"
#include "EntityStats.h"

#include "Engine/World.h"

UStatusEffectTracker::UStatusEffectTracker()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UStatusEffectTracker::EndPlay( const EEndPlayReason::Type endPlayReason )
{
	ClearAll();
	Super::EndPlay( endPlayReason );
}

void UStatusEffectTracker::TickComponent(
	float dt, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction )
{
	Super::TickComponent( dt, tickType, thisTickFunction );

	if ( Active_.Num() == 0 )
	{
		return;
	}

	AActor* owner = GetOwner();
	UWorld* world = GetWorld();
	if ( !owner || !world )
	{
		return;
	}

	if ( const IEntity* entity = Cast<IEntity>( owner ) )
	{
		if ( !entity->Stats().IsAlive() )
		{
			NotifyOwnerDied();
			return;
		}
	}

	const float now = world->GetTimeSeconds();

	for ( int32 i = 0; i < Active_.Num(); ++i )
	{
		FActiveStatus& state = Active_[i];
		if ( !state.Def )
		{
			continue;
		}

		if ( state.Def->GetTickInterval() > 0.f && now >= state.NextTickAt )
		{
			state.Def->OnTick( owner, state );

			if ( !IsValid( this ) || !IsValid( owner ) )
			{
				return;
			}

			if ( !Active_.IsValidIndex( i ) )
			{
				return;
			}
			Active_[i].NextTickAt = now + Active_[i].Def->GetTickInterval();
		}
	}

	bool bRemovedAny = false;
	for ( int32 i = Active_.Num() - 1; i >= 0; --i )
	{
		FActiveStatus& state = Active_[i];
		if ( now < state.ExpiresAt )
		{
			continue;
		}
		if ( state.Def )
		{
			state.Def->OnRemove( owner, state );
		}
		ReleaseStatusVisual( state );
		Active_.RemoveAt( i );
		bRemovedAny = true;
	}

	if ( bRemovedAny )
	{
		RecomputeStackedModifiers();
	}
}

void UStatusEffectTracker::ApplyStatus( UStatusEffectDef* def, AActor* instigator )
{
	if ( !def || def->StatusTag.IsNone() )
	{
		return;
	}

	UWorld* world = GetWorld();
	if ( !world )
	{
		return;
	}
	const float now = world->GetTimeSeconds();

	for ( FActiveStatus& existing : Active_ )
	{
		if ( existing.Def == def )
		{
			existing.ExpiresAt = now + def->Duration;
			if ( instigator )
			{
				existing.Instigator = instigator;
			}
			def->OnReapply( GetOwner(), existing );
			RecomputeStackedModifiers();
			return;
		}
	}

	FActiveStatus state;
	state.Def = def;
	state.ExpiresAt = now + def->Duration;
	state.NextTickAt = def->GetTickInterval() > 0.f ? now + def->GetTickInterval() : MAX_flt;
	state.Instigator = instigator;

	const int32 idx = Active_.Add( MoveTemp( state ) );
	def->OnApply( GetOwner(), Active_[idx] );

	if ( UCardVisualSubsystem* visuals = UCardVisualSubsystem::Get( this ) )
	{
		Active_[idx].VisualHandle = visuals->BeginSticky( def->GetVisualConfig(), GetOwner(), nullptr );
	}

	RecomputeStackedModifiers();
}

bool UStatusEffectTracker::HasStatus( const UStatusEffectDef* def ) const
{
	if ( !def )
	{
		return false;
	}
	return Active_.ContainsByPredicate( [def]( const FActiveStatus& s ) { return s.Def == def; } );
}

bool UStatusEffectTracker::HasStatusTag( FName tag ) const
{
	if ( tag.IsNone() )
	{
		return false;
	}
	return Active_.ContainsByPredicate(
		[tag]( const FActiveStatus& s ) { return s.Def && s.Def->StatusTag == tag; } );
}

void UStatusEffectTracker::RemoveStatus( UStatusEffectDef* def )
{
	if ( !def )
	{
		return;
	}

	bool bRemovedAny = false;
	for ( int32 i = Active_.Num() - 1; i >= 0; --i )
	{
		if ( Active_[i].Def != def )
		{
			continue;
		}
		if ( Active_[i].Def )
		{
			Active_[i].Def->OnRemove( GetOwner(), Active_[i] );
		}
		ReleaseStatusVisual( Active_[i] );
		Active_.RemoveAt( i );
		bRemovedAny = true;
	}

	if ( bRemovedAny )
	{
		RecomputeStackedModifiers();
	}
}

void UStatusEffectTracker::ClearAll()
{
	AActor* owner = GetOwner();
	for ( FActiveStatus& state : Active_ )
	{
		if ( state.Def )
		{
			state.Def->OnRemove( owner, state );
		}
		ReleaseStatusVisual( state );
	}
	Active_.Empty();

	RecomputeStackedModifiers();
}

void UStatusEffectTracker::NotifyOwnerDied()
{
	ClearAll();
	SetComponentTickEnabled( false );
}

void UStatusEffectTracker::ReleaseStatusVisual( FActiveStatus& state )
{
	if ( !state.VisualHandle.IsValid() )
	{
		return;
	}

	if ( UCardVisualSubsystem* visuals = UCardVisualSubsystem::Get( this ) )
	{
		visuals->EndSticky( state.VisualHandle );
	}
	state.VisualHandle.Reset();
}

void UStatusEffectTracker::RecomputeStackedModifiers()
{
	AActor* owner = GetOwner();
	IEntity* entity = Cast<IEntity>( owner );
	if ( !entity )
	{
		return;
	}
	FEntityStats& stats = entity->Stats();

	float slowSum = 0.f;
	float atkSlowSum = 0.f;
	for ( const FActiveStatus& s : Active_ )
	{
		if ( !s.Def )
		{
			continue;
		}
		if ( s.Def->IsA( UStatusEffect_Slow::StaticClass() ) )
		{
			slowSum += s.StackAmount;
		}
		else if ( s.Def->IsA( UStatusEffect_AttackSlow::StaticClass() ) )
		{
			atkSlowSum += s.StackAmount;
		}
	}

	float slowCap = 90.f;
	float atkSlowCap = 90.f;
	if ( UCardSubsystem* cards = UCardSubsystem::Get( this ) )
	{
		if ( UCardPoolConfig* poolConfig = cards->GetPoolConfig() )
		{
			slowCap = FMath::Clamp( poolConfig->GlobalSlowCapPercent, 0.f, 99.f );
			atkSlowCap = FMath::Clamp( poolConfig->GlobalAttackSlowCapPercent, 0.f, 99.f );
		}
	}

	UFollowComponent* follow = owner ? owner->FindComponentByClass<UFollowComponent>() : nullptr;

	if ( slowSum > 0.f )
	{
		if ( CachedOriginalMaxSpeed_ < 0.f )
		{
			CachedOriginalMaxSpeed_ = stats.MaxSpeed();
		}
		const float clamped = FMath::Min( slowSum, slowCap );
		const float newSpeed = CachedOriginalMaxSpeed_ * ( 1.f - clamped / 100.f );
		stats.SetMaxSpeed( newSpeed );
		if ( follow )
		{
			follow->SetMaxSpeed( newSpeed );
		}
	}
	else if ( CachedOriginalMaxSpeed_ >= 0.f )
	{
		stats.SetMaxSpeed( CachedOriginalMaxSpeed_ );
		if ( follow )
		{
			follow->SetMaxSpeed( CachedOriginalMaxSpeed_ );
		}
		CachedOriginalMaxSpeed_ = -1.f;
	}

	if ( atkSlowSum > 0.f )
	{
		if ( CachedOriginalAttackCooldown_ < 0.f )
		{
			CachedOriginalAttackCooldown_ = stats.AttackCooldown();
		}
		const float clamped = FMath::Min( atkSlowSum, atkSlowCap );
		stats.SetAttackCooldown( CachedOriginalAttackCooldown_ * ( 1.f + clamped / 100.f ) );
	}
	else if ( CachedOriginalAttackCooldown_ >= 0.f )
	{
		stats.SetAttackCooldown( CachedOriginalAttackCooldown_ );
		CachedOriginalAttackCooldown_ = -1.f;
	}
}

UStatusEffectTracker* UStatusEffectTracker::EnsureOn( AActor* actor )
{
	if ( !actor )
	{
		return nullptr;
	}

	if ( UStatusEffectTracker* existing = actor->FindComponentByClass<UStatusEffectTracker>() )
	{
		return existing;
	}

	UStatusEffectTracker* tracker = NewObject<UStatusEffectTracker>( actor );
	if ( tracker )
	{
		tracker->RegisterComponent();
	}
	return tracker;
}
