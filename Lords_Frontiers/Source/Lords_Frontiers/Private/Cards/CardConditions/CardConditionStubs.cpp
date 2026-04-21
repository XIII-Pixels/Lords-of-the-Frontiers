#include "Cards/CardConditions/CardConditionStubs.h"

#include "Cards/CardEffectHostComponent.h"

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
