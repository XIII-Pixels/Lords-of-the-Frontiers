#include "Cards/CardEffectHostComponent.h"

#include "Cards/CardDataAsset.h"
#include "Cards/CardEffect.h"

UCardEffectHostComponent::UCardEffectHostComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCardEffectHostComponent::RegisterEffect( UCardDataAsset* card, int32 eventIndex, UCardEffect* effect )
{
	if ( !card || !effect )
	{
		return;
	}

	FRegisteredCardEffect record;
	record.SourceCard = card;
	record.EventIndex = eventIndex;
	record.Effect = effect;
	Active_.Add( record );
}

void UCardEffectHostComponent::UnregisterBySourceCard( UCardDataAsset* card )
{
	if ( !card )
	{
		return;
	}

	Active_.RemoveAll( [card]( const FRegisteredCardEffect& r )
	{
		return r.SourceCard == card;
	} );
}

void UCardEffectHostComponent::ClearAll()
{
	Active_.Empty();
	Counters_.Empty();
}

int32 UCardEffectHostComponent::GetCounter( FName key ) const
{
	if ( const int32* value = Counters_.Find( key ) )
	{
		return *value;
	}
	return 0;
}

void UCardEffectHostComponent::SetCounter( FName key, int32 value )
{
	Counters_.FindOrAdd( key ) = value;
}

int32 UCardEffectHostComponent::IncrementCounter( FName key )
{
	int32& slot = Counters_.FindOrAdd( key );
	slot += 1;
	return slot;
}

FName UCardEffectHostComponent::MakeCounterKey( const UCardDataAsset* card, int32 eventIndex, FName localTag )
{
	const FString cardName = card ? card->GetName() : TEXT( "null" );
	return FName( *FString::Printf( TEXT( "%s#%d#%s" ), *cardName, eventIndex, *localTag.ToString() ) );
}
