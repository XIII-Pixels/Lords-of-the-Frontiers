#include "Cards/CardEffects/CardEffect_StackingPrimeDamageBonus.h"

#include "Building/Building.h"
#include "Cards/CardDataAsset.h"
#include "Cards/CardEffectHostComponent.h"
#include "Components/Attack/AttackRangedComponent.h"

namespace
{
	FName MakeStackKey( const UCardDataAsset* card )
	{
		const FString cardName = card ? card->GetName() : TEXT( "null" );
		return FName( *FString::Printf( TEXT( "%s#prime_stacks" ), *cardName ) );
	}
}

void UCardEffect_StackingPrimeDamageBonus::Execute_Implementation( const FCardEffectContext& context )
{
	UCardEffectHostComponent* host = context.EffectHost.Get();
	ABuilding* building = context.Building.Get();
	if ( !host || !building )
	{
		return;
	}

	const FName stackKey = MakeStackKey( context.SourceCard.Get() );

	switch ( context.TriggerReason )
	{
	case ECardTriggerReason::BeforeAttackFire:
	{
		const int32 stacks = host->GetCounter( stackKey );
		if ( stacks <= 0 )
		{
			return;
		}
		UAttackRangedComponent* attack = building->FindComponentByClass<UAttackRangedComponent>();
		if ( attack )
		{
			attack->AddPendingDamageBonus( static_cast<float>( stacks ) * StepPerStackPercent );
		}
		return;
	}

	case ECardTriggerReason::AttackFired:
	case ECardTriggerReason::HitLanded:
	{
		const int32 maxStacks = FMath::Max( 1, MaxStacks );
		const int32 current = host->GetCounter( stackKey );
		if ( current < maxStacks )
		{
			host->SetCounter( stackKey, current + 1 );
		}
		return;
	}

	case ECardTriggerReason::TargetChanged:
	{
		if ( bResetOnTargetChange )
		{
			host->SetCounter( stackKey, 0 );
		}
		return;
	}

	default:
		return;
	}
}

void UCardEffect_StackingPrimeDamageBonus::Revert_Implementation( const FCardEffectContext& context )
{
	UCardEffectHostComponent* host = context.EffectHost.Get();
	if ( !host )
	{
		return;
	}
	host->SetCounter( MakeStackKey( context.SourceCard.Get() ), 0 );
}

FText UCardEffect_StackingPrimeDamageBonus::GetDisplayText_Implementation() const
{
	const float cap = StepPerStackPercent * static_cast<float>( MaxStacks );
	return FText::FromString(
		FString::Printf( TEXT( "+%g%% damage per stack (max %d × %g%% = %g%%)" ),
			StepPerStackPercent, MaxStacks, StepPerStackPercent, cap ) );
}
