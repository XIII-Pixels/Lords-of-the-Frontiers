#include "Cards/CardPoolConfig.h"

#include "Cards/CardDataAsset.h"
#include "Cards/CardRarityPoolConfig.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#include "UObject/Class.h"
#endif

int32 UCardPoolConfig::GetPoolSize() const
{
	int32 total = 0;
	for ( const TObjectPtr<UCardRarityPoolConfig>& rarityPool : RarityPools )
	{
		if ( rarityPool )
		{
			total += rarityPool->Cards.Num();
		}
	}
	return total;
}

#if WITH_EDITOR
EDataValidationResult UCardPoolConfig::IsDataValid( FDataValidationContext& context ) const
{
	EDataValidationResult result = Super::IsDataValid( context );

	if ( RarityPools.Num() == 0 )
	{
		context.AddError( FText::FromString( TEXT( "RarityPools is empty - no cards available for selection" ) ) );
		result = EDataValidationResult::Invalid;
	}

	const int32 totalCards = GetPoolSize();
	if ( totalCards < CardsToOffer )
	{
		context.AddWarning( FText::FromString(
			FString::Printf( TEXT( "RarityPools contain %d cards but CardsToOffer is %d" ), totalCards, CardsToOffer )
		) );
	}

	if ( CardsToSelect > CardsToOffer )
	{
		context.AddError( FText::FromString( TEXT( "CardsToSelect cannot be greater than CardsToOffer" ) ) );
		result = EDataValidationResult::Invalid;
	}

	const UEnum* rarityEnum = StaticEnum<ECardRarity>();
	TSet<ECardRarity> seenRarities;
	TSet<FName> seenIDs;
	int32 nullPools = 0;

	for ( const TObjectPtr<UCardRarityPoolConfig>& rarityPool : RarityPools )
	{
		if ( !rarityPool )
		{
			++nullPools;
			continue;
		}

		if ( seenRarities.Contains( rarityPool->Rarity ) )
		{
			context.AddWarning( FText::FromString( FString::Printf(
				TEXT( "Duplicate rarity tier %s in RarityPools" ),
				rarityEnum ? *rarityEnum->GetNameStringByValue( static_cast<int64>( rarityPool->Rarity ) ) : TEXT( "?" )
			) ) );
		}
		seenRarities.Add( rarityPool->Rarity );

		for ( const TObjectPtr<UCardDataAsset>& card : rarityPool->Cards )
		{
			if ( card && !card->CardID.IsNone() )
			{
				if ( seenIDs.Contains( card->CardID ) )
				{
					context.AddWarning( FText::FromString(
						FString::Printf( TEXT( "Duplicate CardID across RarityPools: %s" ), *card->CardID.ToString() )
					) );
				}
				seenIDs.Add( card->CardID );
			}
		}
	}

	if ( nullPools > 0 )
	{
		context.AddWarning( FText::FromString(
			FString::Printf( TEXT( "RarityPools contains %d null entries" ), nullPools )
		) );
	}

	const int32 maxOfferable = seenRarities.Num() * MaxCardsPerRarityInOffering;
	if ( maxOfferable > 0 && maxOfferable < CardsToOffer )
	{
		context.AddWarning( FText::FromString( FString::Printf(
			TEXT( "%d unique rarities * MaxCardsPerRarityInOffering (%d) = %d, but CardsToOffer is %d" ),
			seenRarities.Num(), MaxCardsPerRarityInOffering, maxOfferable, CardsToOffer
		) ) );
	}

	return result;
}
#endif
