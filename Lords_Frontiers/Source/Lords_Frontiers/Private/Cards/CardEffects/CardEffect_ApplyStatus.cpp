#include "Cards/CardEffects/CardEffect_ApplyStatus.h"

#include "Building/Building.h"
#include "Cards/StatusEffects/StatusEffectDef.h"
#include "Cards/StatusEffects/StatusEffectTracker.h"

void UCardEffect_ApplyStatus::Execute_Implementation( const FCardEffectContext& context )
{
	if ( context.TriggerReason != ECardTriggerReason::HitLanded &&
	     context.TriggerReason != ECardTriggerReason::AttackFired )
	{
		return;
	}

	if ( !StatusToApply )
	{
		return;
	}

	AActor* target = context.EventInstigator.Get();
	if ( !target )
	{
		return;
	}

	UStatusEffectTracker* tracker = UStatusEffectTracker::EnsureOn( target );
	if ( tracker )
	{
		tracker->ApplyStatus( StatusToApply, context.Building.Get() );
	}
}

FText UCardEffect_ApplyStatus::GetDisplayText_Implementation() const
{
	if ( !StatusToApply )
	{
		return FText::GetEmpty();
	}
	return FText::FromString(
		FString::Printf( TEXT( "Apply %s" ), *StatusToApply->StatusTag.ToString() ) );
}
