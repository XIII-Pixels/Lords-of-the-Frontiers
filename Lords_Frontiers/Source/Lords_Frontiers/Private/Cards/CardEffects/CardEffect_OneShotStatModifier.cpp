#include "Cards/CardEffects/CardEffect_OneShotStatModifier.h"

#include "Building/Building.h"
#include "Cards/CardDataAsset.h"
#include "Cards/CardEffectHostComponent.h"
#include "Cards/CardEffects/CardEffect_StatModifier.h"
#include "Cards/CardEffects/StatReflectionHelpers.h"
#include "EntityStats.h"

namespace
{
	FName MakeShotsKey( const UCardDataAsset* card )
	{
		const FString cardName = card ? card->GetName() : TEXT( "null" );
		return FName( *FString::Printf( TEXT( "%s#oneshot_counter" ), *cardName ) );
	}

	FName MakeOneShotAppliedKey( const UCardDataAsset* card )
	{
		const FString cardName = card ? card->GetName() : TEXT( "null" );
		return FName( *FString::Printf( TEXT( "%s#oneshot_applied" ), *cardName ) );
	}
}

void UCardEffect_OneShotStatModifier::Execute_Implementation( const FCardEffectContext& context )
{
	if ( context.TriggerReason != ECardTriggerReason::AttackFired )
	{
		return;
	}

	UCardEffectHostComponent* host = context.EffectHost.Get();
	ABuilding* building = context.Building.Get();
	if ( !host || !building || StatName.IsNone() )
	{
		return;
	}

	const FName shotsKey = MakeShotsKey( context.SourceCard.Get() );
	const FName appliedKey = MakeOneShotAppliedKey( context.SourceCard.Get() );

	const int32 previousApplied = host->GetCounter( appliedKey );
	if ( previousApplied != 0 )
	{
		CardStatReflection::ApplyStatDelta( building, StatName, -static_cast<float>( previousApplied ) / 1000.f );
		host->SetCounter( appliedKey, 0 );
	}

	const int32 shots = host->IncrementCounter( shotsKey );
	const int32 every = FMath::Max( 1, ApplyEveryN );
	if ( shots % every != 0 )
	{
		return;
	}

	const float applied = CardStatReflection::ApplyStatDelta( building, StatName, Delta );
	if ( !FMath::IsNearlyZero( applied ) )
	{
		host->SetCounter( appliedKey, FMath::RoundToInt( applied * 1000.f ) );
	}
}

void UCardEffect_OneShotStatModifier::Revert_Implementation( const FCardEffectContext& context )
{
	UCardEffectHostComponent* host = context.EffectHost.Get();
	ABuilding* building = context.Building.Get();
	if ( !host || !building )
	{
		return;
	}

	const FName appliedKey = MakeOneShotAppliedKey( context.SourceCard.Get() );
	const int32 previousApplied = host->GetCounter( appliedKey );
	if ( previousApplied != 0 )
	{
		CardStatReflection::ApplyStatDelta( building, StatName, -static_cast<float>( previousApplied ) / 1000.f );
		host->SetCounter( appliedKey, 0 );
	}
}

FText UCardEffect_OneShotStatModifier::GetDisplayText_Implementation() const
{
	if ( StatName.IsNone() )
	{
		return FText::GetEmpty();
	}

	FString displayName = StatName.ToString();
	if ( FProperty* prop = FEntityStats::StaticStruct()->FindPropertyByName( StatName ) )
	{
		displayName = prop->GetDisplayNameText().ToString();
	}

	return FText::FromString(
		FString::Printf( TEXT( "%+g %s every %d shot(s), reverts next shot" ),
			Delta, *displayName, ApplyEveryN ) );
}

TArray<FString> UCardEffect_OneShotStatModifier::GetModifiableStatNames()
{
	return UCardEffect_StatModifier::GetModifiableStatNames();
}
