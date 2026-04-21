#include "Cards/StatusEffects/StatusEffectDef.h"

#include "Cards/StatusEffects/StatusEffectTracker.h"
#include "Entity.h"
#include "EntityStats.h"

UStatusEffect_Burn::UStatusEffect_Burn()
{
	StatusTag = FName( TEXT( "Status.Burn" ) );
}

void UStatusEffect_Burn::OnTick( AActor* owner, FActiveStatus& state ) const
{
	if ( IEntity* entity = Cast<IEntity>( owner ) )
	{
		if ( entity->Stats().IsAlive() )
		{
			entity->TakeDamage( DamagePerTick );
		}
	}
}

UStatusEffect_Slow::UStatusEffect_Slow()
{
	StatusTag = FName( TEXT( "Status.Slow" ) );
}

void UStatusEffect_Slow::OnApply( AActor* owner, FActiveStatus& state ) const
{
	IEntity* entity = Cast<IEntity>( owner );
	if ( !entity )
	{
		return;
	}

	FEntityStats& stats = entity->Stats();
	state.CachedOriginal = stats.MaxSpeed();

	const float reduction = FMath::Clamp( SpeedReductionPercent, 0.f, 99.f );
	const float newSpeed = state.CachedOriginal * ( 1.f - reduction / 100.f );
	stats.SetMaxSpeed( newSpeed );
}

void UStatusEffect_Slow::OnRemove( AActor* owner, FActiveStatus& state ) const
{
	IEntity* entity = Cast<IEntity>( owner );
	if ( !entity )
	{
		return;
	}
	entity->Stats().SetMaxSpeed( state.CachedOriginal );
}
