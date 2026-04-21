#include "Cards/CardEffects/CardEffect_StackingBuff.h"

#include "Building/Building.h"
#include "Cards/CardEffectHostComponent.h"
#include "Cards/CardEffects/CardEffect_StatModifier.h"
#include "EntityStats.h"

#include "UObject/UnrealType.h"

namespace
{
	const FName StacksTag( TEXT( "stacks" ) );
	const FName AccumulatedTag( TEXT( "accum" ) );

	FNumericProperty* FindNumericProperty( FName statName )
	{
		if ( statName.IsNone() )
		{
			return nullptr;
		}
		FProperty* prop = FEntityStats::StaticStruct()->FindPropertyByName( statName );
		return CastField<FNumericProperty>( prop );
	}

	void ApplyStatDelta( ABuilding* building, FName statName, float delta )
	{
		if ( !building || FMath::IsNearlyZero( delta ) )
		{
			return;
		}
		FNumericProperty* prop = FindNumericProperty( statName );
		if ( !prop )
		{
			return;
		}
		FEntityStats& stats = building->Stats();
		void* addr = prop->ContainerPtrToValuePtr<void>( &stats );
		const double current = prop->GetFloatingPointPropertyValue( addr );
		prop->SetFloatingPointPropertyValue( addr, current + delta );
	}
}

void UCardEffect_StackingBuff::Execute_Implementation( const FCardEffectContext& context )
{
	UCardEffectHostComponent* host = context.EffectHost.Get();
	ABuilding* building = context.Building.Get();
	if ( !host || !building || StatName.IsNone() )
	{
		return;
	}

	const FName accumKey = UCardEffectHostComponent::MakeCounterKey(
		context.SourceCard.Get(), context.EventIndex, AccumulatedTag );

	if ( context.TriggerReason == ECardTriggerReason::TargetChanged )
	{
		if ( !bResetOnTargetChange )
		{
			return;
		}
		const int32 accumMilli = host->GetCounter( accumKey );
		if ( accumMilli != 0 )
		{
			ApplyStatDelta( building, StatName, -static_cast<float>( accumMilli ) / 1000.f );
			host->SetCounter( accumKey, 0 );
		}
		return;
	}

	if ( context.TriggerReason != ECardTriggerReason::AttackFired )
	{
		return;
	}

	const int32 accumMilli = host->GetCounter( accumKey );
	const float accumulated = static_cast<float>( accumMilli ) / 1000.f;

	if ( MaxAccumulated > 0.f && accumulated + 1e-3f >= MaxAccumulated )
	{
		return;
	}

	float step = StepPerTrigger;
	if ( MaxAccumulated > 0.f && accumulated + step > MaxAccumulated )
	{
		step = MaxAccumulated - accumulated;
	}

	if ( FMath::IsNearlyZero( step ) )
	{
		return;
	}

	ApplyStatDelta( building, StatName, step );
	host->SetCounter( accumKey, accumMilli + FMath::RoundToInt( step * 1000.f ) );
}

void UCardEffect_StackingBuff::Revert_Implementation( const FCardEffectContext& context )
{
	UCardEffectHostComponent* host = context.EffectHost.Get();
	ABuilding* building = context.Building.Get();
	if ( !host || !building )
	{
		return;
	}

	const FName accumKey = UCardEffectHostComponent::MakeCounterKey(
		context.SourceCard.Get(), context.EventIndex, AccumulatedTag );

	const int32 accumMilli = host->GetCounter( accumKey );
	if ( accumMilli != 0 )
	{
		ApplyStatDelta( building, StatName, -static_cast<float>( accumMilli ) / 1000.f );
		host->SetCounter( accumKey, 0 );
	}
}

FText UCardEffect_StackingBuff::GetDisplayText_Implementation() const
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

	const TCHAR* resetTail = bResetOnTargetChange ? TEXT( ", reset on target change" ) : TEXT( "" );
	if ( MaxAccumulated > 0.f )
	{
		return FText::FromString( FString::Printf(
			TEXT( "+%g %s per trigger (max %g%s)" ),
			StepPerTrigger, *displayName, MaxAccumulated, resetTail ) );
	}
	return FText::FromString( FString::Printf(
		TEXT( "+%g %s per trigger%s" ),
		StepPerTrigger, *displayName, resetTail ) );
}

TArray<FString> UCardEffect_StackingBuff::GetModifiableStatNames()
{
	return UCardEffect_StatModifier::GetModifiableStatNames();
}
