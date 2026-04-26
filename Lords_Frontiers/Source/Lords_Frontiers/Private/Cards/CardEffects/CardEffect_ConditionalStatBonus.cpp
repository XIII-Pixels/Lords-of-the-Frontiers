#include "Cards/CardEffects/CardEffect_ConditionalStatBonus.h"

#include "Building/Building.h"
#include "Cards/CardCondition.h"
#include "Cards/CardDataAsset.h"
#include "Cards/CardEffectHostComponent.h"
#include "Cards/CardEffects/CardEffect_StatModifier.h"
#include "Cards/CardEffects/StatReflectionHelpers.h"
#include "EntityStats.h"

namespace
{
	FName MakeCondAppliedKey( const UCardDataAsset* card )
	{
		const FString cardName = card ? card->GetName() : TEXT( "null" );
		return FName( *FString::Printf( TEXT( "%s#cond_applied" ), *cardName ) );
	}
}

void UCardEffect_ConditionalStatBonus::Execute_Implementation( const FCardEffectContext& context )
{
	if ( context.TriggerReason != ECardTriggerReason::AuraTick )
	{
		return;
	}

	UCardEffectHostComponent* host = context.EffectHost.Get();
	ABuilding* building = context.Building.Get();
	if ( !host || !building || StatName.IsNone() )
	{
		return;
	}

	const bool bConditionMet = ActiveWhile ? ActiveWhile->IsMet( context ) : true;

	const FName key = MakeCondAppliedKey( context.SourceCard.Get() );
	const int32 appliedMilli = host->GetCounter( key );
	const bool bApplied = appliedMilli != 0;

	if ( bConditionMet && !bApplied && !FMath::IsNearlyZero( BonusAmount ) )
	{
		const float delta = CardStatReflection::ApplyStatDelta( building, StatName, BonusAmount );
		if ( !FMath::IsNearlyZero( delta ) )
		{
			host->SetCounter( key, FMath::RoundToInt( delta * 1000.f ) );
		}
	}
	else if ( !bConditionMet && bApplied )
	{
		CardStatReflection::ApplyStatDelta( building, StatName, -static_cast<float>( appliedMilli ) / 1000.f );
		host->SetCounter( key, 0 );
	}
}

void UCardEffect_ConditionalStatBonus::Revert_Implementation( const FCardEffectContext& context )
{
	UCardEffectHostComponent* host = context.EffectHost.Get();
	ABuilding* building = context.Building.Get();
	if ( !host || !building )
	{
		return;
	}

	const FName key = MakeCondAppliedKey( context.SourceCard.Get() );
	const int32 appliedMilli = host->GetCounter( key );
	if ( appliedMilli != 0 )
	{
		CardStatReflection::ApplyStatDelta( building, StatName, -static_cast<float>( appliedMilli ) / 1000.f );
		host->SetCounter( key, 0 );
	}
}

FText UCardEffect_ConditionalStatBonus::GetDisplayText_Implementation() const
{
	if ( StatName.IsNone() )
	{
		return FText::GetEmpty();
	}

	FString displayName = CardStatReflection::GetStatDisplayName( StatName );
	if ( displayName.IsEmpty() )
	{
		displayName = StatName.ToString();
	}

	return FText::FromString(
		FString::Printf( TEXT( "%+g %s while condition" ), BonusAmount, *displayName ) );
}

TArray<FString> UCardEffect_ConditionalStatBonus::GetModifiableStatNames()
{
	return UCardEffect_StatModifier::GetModifiableStatNames();
}
