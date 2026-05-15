#include "Cards/StatusEffects/StatusEffectDef.h"

#include "Cards/StatusEffects/StatusEffectTracker.h"
#include "Cards/Visuals/CardVFXAsset.h"
#include "Core/Subsystems/SessionLogger/DamageEvent.h"
#include "Entity.h"
#include "EntityStats.h"

const FCardVisualConfig& UStatusEffectDef::GetVisualConfig() const
{
	if ( VFX )
	{
		return VFX->Config;
	}
	return VisualConfig;
}

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

	if ( instigator && DamagePerTick > 0 )
	{
		FDamageEvents::OnDamageDealt.Broadcast( instigator, owner, DamagePerTick, false );
	}

	entity->TakeDamage( DamagePerTick, instigator );
}

UStatusEffect_Slow::UStatusEffect_Slow()
{
	StatusTag = FName( TEXT( "Status.Slow" ) );
}

void UStatusEffect_Slow::OnApply( AActor* /*owner*/, FActiveStatus& state ) const
{
	state.StackAmount = FMath::Clamp( SpeedReductionPercent, 0.f, 99.f );
}

void UStatusEffect_Slow::OnReapply( AActor* /*owner*/, FActiveStatus& state ) const
{
	if ( !bStackable )
	{
		return;
	}
	const float cap = FMath::Clamp( MaxStackedPercent, 0.f, 99.f );
	state.StackAmount = FMath::Min( cap, state.StackAmount + SpeedReductionPercent );
}

void UStatusEffect_Slow::OnRemove( AActor* /*owner*/, FActiveStatus& state ) const
{
	state.StackAmount = 0.f;
}

UStatusEffect_AttackSlow::UStatusEffect_AttackSlow()
{
	StatusTag = FName( TEXT( "Status.AttackSlow" ) );
}

void UStatusEffect_AttackSlow::OnApply( AActor* /*owner*/, FActiveStatus& state ) const
{
	state.StackAmount = FMath::Clamp( AttackSpeedReductionPercent, 0.f, 99.f );
}

void UStatusEffect_AttackSlow::OnReapply( AActor* /*owner*/, FActiveStatus& state ) const
{
	if ( !bStackable )
	{
		return;
	}
	const float cap = FMath::Clamp( MaxStackedPercent, 0.f, 99.f );
	state.StackAmount = FMath::Min( cap, state.StackAmount + AttackSpeedReductionPercent );
}

void UStatusEffect_AttackSlow::OnRemove( AActor* /*owner*/, FActiveStatus& state ) const
{
	state.StackAmount = 0.f;
}
