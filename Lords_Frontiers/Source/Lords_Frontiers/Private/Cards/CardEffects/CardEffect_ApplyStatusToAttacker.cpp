#include "Cards/CardEffects/CardEffect_ApplyStatusToAttacker.h"

#include "Cards/StatusEffects/StatusEffectDef.h"
#include "Cards/StatusEffects/StatusEffectTracker.h"
#include "Entity.h"
#include "EntityStats.h"

void UCardEffect_ApplyStatusToAttacker::Execute_Implementation( const FCardEffectContext& context )
{
	if ( context.TriggerReason != ECardTriggerReason::Damaged )
	{
		return;
	}

	if ( !StatusToApply )
	{
		return;
	}

	AActor* attacker = context.EventInstigator.Get();
	if ( !attacker )
	{
		return;
	}

	const IEntity* attackerEntity = Cast<IEntity>( attacker );
	if ( !attackerEntity || !attackerEntity->Stats().IsAlive() )
	{
		return;
	}

	if ( UStatusEffectTracker* tracker = UStatusEffectTracker::EnsureOn( attacker ) )
	{
		tracker->ApplyStatus( StatusToApply );
	}
}

FText UCardEffect_ApplyStatusToAttacker::GetDisplayText_Implementation() const
{
	if ( !StatusToApply )
	{
		return FText::FromString( TEXT( "Apply status to attacker" ) );
	}
	return FText::FromString(
		FString::Printf( TEXT( "Apply %s to attacker" ), *StatusToApply->StatusTag.ToString() ) );
}
