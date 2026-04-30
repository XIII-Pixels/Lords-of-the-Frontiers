#include "Cards/CardConditions/CardConditionStubs.h"

#include "Building/Building.h"
#include "Cards/CardEffectHostComponent.h"
#include "Cards/StatusEffects/StatusEffectDef.h"
#include "Cards/StatusEffects/StatusEffectTracker.h"
#include "Entity.h"
#include "EntityStats.h"

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

namespace
{
	FName StatusKindToTag( ECardStatusKind kind )
	{
		switch ( kind )
		{
		case ECardStatusKind::Burn:       return FName( TEXT( "Status.Burn" ) );
		case ECardStatusKind::Slow:       return FName( TEXT( "Status.Slow" ) );
		case ECardStatusKind::AttackSlow: return FName( TEXT( "Status.AttackSlow" ) );
		default:                          return NAME_None;
		}
	}
}

bool UCardCondition_TargetHasStatus::IsMet_Implementation( const FCardEffectContext& context ) const
{
	AActor* target = context.EventInstigator.Get();
	if ( !target )
	{
		return false;
	}

	const UStatusEffectTracker* tracker = target->FindComponentByClass<UStatusEffectTracker>();
	if ( !tracker )
	{
		return false;
	}

	if ( CustomStatus )
	{
		return tracker->HasStatus( CustomStatus );
	}

	if ( StatusKind == ECardStatusKind::AnySlow )
	{
		return tracker->HasStatusTag( FName( TEXT( "Status.Slow" ) ) )
			|| tracker->HasStatusTag( FName( TEXT( "Status.AttackSlow" ) ) );
	}

	const FName tag = StatusKindToTag( StatusKind );
	return !tag.IsNone() && tracker->HasStatusTag( tag );
}

FText UCardCondition_TargetHasStatus::GetDisplayText_Implementation() const
{
	if ( CustomStatus )
	{
		return FText::FromString(
			FString::Printf( TEXT( "Target has %s" ), *CustomStatus->StatusTag.ToString() ) );
	}
	const TCHAR* name = TEXT( "?" );
	switch ( StatusKind )
	{
	case ECardStatusKind::Burn:       name = TEXT( "Burn" ); break;
	case ECardStatusKind::Slow:       name = TEXT( "Slow" ); break;
	case ECardStatusKind::AttackSlow: name = TEXT( "Attack Slow" ); break;
	case ECardStatusKind::AnySlow:    name = TEXT( "Any Slow" ); break;
	}
	return FText::FromString( FString::Printf( TEXT( "Target has %s" ), name ) );
}

namespace
{
	bool IsBelowThreshold( const AActor* actor, int32 thresholdPercent )
	{
		if ( !actor )
		{
			return false;
		}
		const IEntity* entity = Cast<IEntity>( actor );
		if ( !entity )
		{
			return false;
		}
		const FEntityStats& stats = entity->Stats();
		if ( stats.MaxHealth() <= 0 )
		{
			return false;
		}
		const int32 percent = FMath::RoundToInt(
			100.f * static_cast<float>( stats.Health() ) / static_cast<float>( stats.MaxHealth() ) );
		return percent < thresholdPercent;
	}
}

bool UCardCondition_TargetLowHP::IsMet_Implementation( const FCardEffectContext& context ) const
{
	return IsBelowThreshold( context.EventInstigator.Get(), ThresholdPercent );
}

bool UCardCondition_OwnerLowHP::IsMet_Implementation( const FCardEffectContext& context ) const
{
	return IsBelowThreshold( context.Building.Get(), ThresholdPercent );
}
