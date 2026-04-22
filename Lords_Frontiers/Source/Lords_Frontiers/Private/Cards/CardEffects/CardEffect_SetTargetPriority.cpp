#include "Cards/CardEffects/CardEffect_SetTargetPriority.h"

#include "Building/Building.h"
#include "Cards/CardDataAsset.h"
#include "Cards/CardEffectHostComponent.h"
#include "Components/Attack/AttackRangedComponent.h"

namespace
{
	FName MakePrevKey( const UCardDataAsset* card )
	{
		const FString cardName = card ? card->GetName() : TEXT( "null" );
		return FName( *FString::Printf( TEXT( "%s#prev_target_priority" ), *cardName ) );
	}
}

void UCardEffect_SetTargetPriority::Apply_Implementation( const FCardEffectContext& context )
{
	ABuilding* building = context.Building.Get();
	if ( !building )
	{
		return;
	}

	UAttackRangedComponent* attack = building->FindComponentByClass<UAttackRangedComponent>();
	if ( !attack )
	{
		return;
	}

	if ( UCardEffectHostComponent* host = context.EffectHost.Get() )
	{
		host->SetCounter( MakePrevKey( context.SourceCard.Get() ),
			static_cast<int32>( attack->GetTargetPriority() ) + 1 );
	}

	attack->SetTargetPriority( Priority );
}

void UCardEffect_SetTargetPriority::Revert_Implementation( const FCardEffectContext& context )
{
	ABuilding* building = context.Building.Get();
	if ( !building )
	{
		return;
	}

	UAttackRangedComponent* attack = building->FindComponentByClass<UAttackRangedComponent>();
	if ( !attack )
	{
		return;
	}

	ETowerTargetPriority restored = ETowerTargetPriority::Closest;
	if ( UCardEffectHostComponent* host = context.EffectHost.Get() )
	{
		const int32 stored = host->GetCounter( MakePrevKey( context.SourceCard.Get() ) );
		if ( stored > 0 )
		{
			restored = static_cast<ETowerTargetPriority>( stored - 1 );
			host->SetCounter( MakePrevKey( context.SourceCard.Get() ), 0 );
		}
	}

	attack->SetTargetPriority( restored );
}

FText UCardEffect_SetTargetPriority::GetDisplayText_Implementation() const
{
	const TCHAR* name = TEXT( "Closest" );
	switch ( Priority )
	{
	case ETowerTargetPriority::LowestHP: name = TEXT( "Lowest HP" ); break;
	case ETowerTargetPriority::HighestHP: name = TEXT( "Highest HP" ); break;
	default: break;
	}
	return FText::FromString( FString::Printf( TEXT( "Target priority: %s" ), name ) );
}
