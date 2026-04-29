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
	IEntity* entity = Cast<IEntity>( owner );
	if ( !entity || !entity->Stats().IsAlive() )
	{
		return;
	}

	AActor* instigator = state.Instigator.Get();
	entity->TakeDamage( DamagePerTick, instigator );
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
	state.StackAmount = FMath::Clamp( SpeedReductionPercent, 0.f, 99.f );

	const float newSpeed = state.CachedOriginal * ( 1.f - state.StackAmount / 100.f );
	stats.SetMaxSpeed( newSpeed );
}

void UStatusEffect_Slow::OnReapply( AActor* owner, FActiveStatus& state ) const
{
	if ( !bStackable )
	{
		return;
	}

	IEntity* entity = Cast<IEntity>( owner );
	if ( !entity )
	{
		return;
	}

	const float cap = FMath::Clamp( MaxStackedPercent, 0.f, 99.f );
	state.StackAmount = FMath::Min( cap, state.StackAmount + SpeedReductionPercent );

	FEntityStats& stats = entity->Stats();
	const float newSpeed = state.CachedOriginal * ( 1.f - state.StackAmount / 100.f );
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

UStatusEffect_AttackSlow::UStatusEffect_AttackSlow()
{
	StatusTag = FName( TEXT( "Status.AttackSlow" ) );
}

void UStatusEffect_AttackSlow::OnApply( AActor* owner, FActiveStatus& state ) const
{
	IEntity* entity = Cast<IEntity>( owner );
	if ( !entity )
	{
		return;
	}

	FEntityStats& stats = entity->Stats();
	state.CachedOriginal = stats.AttackCooldown();
	state.StackAmount = FMath::Clamp( AttackSpeedReductionPercent, 0.f, 99.f );

	const float newCooldown = state.CachedOriginal * ( 1.f + state.StackAmount / 100.f );
	stats.SetAttackCooldown( newCooldown );
}

void UStatusEffect_AttackSlow::OnReapply( AActor* owner, FActiveStatus& state ) const
{
	if ( !bStackable )
	{
		return;
	}

	IEntity* entity = Cast<IEntity>( owner );
	if ( !entity )
	{
		return;
	}

	const float cap = FMath::Clamp( MaxStackedPercent, 0.f, 99.f );
	state.StackAmount = FMath::Min( cap, state.StackAmount + AttackSpeedReductionPercent );

	FEntityStats& stats = entity->Stats();
	const float newCooldown = state.CachedOriginal * ( 1.f + state.StackAmount / 100.f );
	stats.SetAttackCooldown( newCooldown );
}

void UStatusEffect_AttackSlow::OnRemove( AActor* owner, FActiveStatus& state ) const
{
	IEntity* entity = Cast<IEntity>( owner );
	if ( !entity )
	{
		return;
	}
	entity->Stats().SetAttackCooldown( state.CachedOriginal );
}
