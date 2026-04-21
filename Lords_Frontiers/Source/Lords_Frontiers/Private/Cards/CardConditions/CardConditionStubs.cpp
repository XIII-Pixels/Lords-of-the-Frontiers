#include "Cards/CardConditions/CardConditionStubs.h"

#include "Cards/CardEffectHostComponent.h"
#include "Cards/StatusEffects/StatusEffectDef.h"
#include "Cards/StatusEffects/StatusEffectTracker.h"

namespace
{
	const FName ShotsCounterTag( TEXT( "shots_counter" ) );
}

bool UCardCondition_AfterNShots::IsMet_Implementation( const FCardEffectContext& context ) const
{
	if ( context.TriggerReason != ECardTriggerReason::AttackFired )
	{
		return false;
	}

	UCardEffectHostComponent* host = context.EffectHost.Get();
	if ( !host )
	{
		return false;
	}

	const FName key = UCardEffectHostComponent::MakeCounterKey(
		context.SourceCard.Get(), context.EventIndex, ShotsCounterTag );

	const int32 count = host->IncrementCounter( key );
	const int32 divisor = FMath::Max( 1, N );
	return ( count % divisor ) == 0;
}

bool UCardCondition_TargetHasStatus::IsMet_Implementation( const FCardEffectContext& context ) const
{
	if ( !RequiredStatus )
	{
		return false;
	}

	AActor* target = context.EventInstigator.Get();
	if ( !target )
	{
		return false;
	}

	if ( const UStatusEffectTracker* tracker = target->FindComponentByClass<UStatusEffectTracker>() )
	{
		return tracker->HasStatus( RequiredStatus );
	}
	return false;
}

FText UCardCondition_TargetHasStatus::GetDisplayText_Implementation() const
{
	if ( !RequiredStatus )
	{
		return FText::FromString( TEXT( "Target has status" ) );
	}
	return FText::FromString(
		FString::Printf( TEXT( "Target has %s" ), *RequiredStatus->StatusTag.ToString() ) );
}
