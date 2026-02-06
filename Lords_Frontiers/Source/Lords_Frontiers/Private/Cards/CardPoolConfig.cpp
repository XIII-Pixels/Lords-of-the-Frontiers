#include "Cards/CardPoolConfig.h"

#include "Cards/CardDataAsset.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#if WITH_EDITOR
EDataValidationResult UCardPoolConfig::IsDataValid( FDataValidationContext& context ) const
{
	EDataValidationResult result = Super::IsDataValid( context );

	if ( CardPool.Num() == 0 )
	{
		context.AddError( FText::FromString( TEXT( "CardPool is empty - no cards available for selection" ) ) );
		result = EDataValidationResult::Invalid;
	}

	if ( CardPool.Num() < CardsToOffer )
	{
		context.AddWarning( FText::FromString( 
			FString::Printf( TEXT( "CardPool has %d cards but CardsToOffer is %d" ), CardPool.Num(), CardsToOffer )
		) );
	}

	if ( CardsToSelect > CardsToOffer )
	{
		context.AddError( FText::FromString( TEXT( "CardsToSelect cannot be greater than CardsToOffer" ) ) );
		result = EDataValidationResult::Invalid;
	}

	int32 nullCount = 0;
	for ( const TObjectPtr<UCardDataAsset>& card : CardPool )
	{
		if ( !card )
		{
			nullCount++;
		}
	}

	if ( nullCount > 0 )
	{
		context.AddWarning( FText::FromString( 
			FString::Printf( TEXT( "CardPool contains %d null entries" ), nullCount )
		) );
	}

	TSet<FName> seenIDs;
	for ( const TObjectPtr<UCardDataAsset>& card : CardPool )
	{
		if ( card && !card->CardID.IsNone() )
		{
			if ( seenIDs.Contains( card->CardID ) )
			{
				context.AddWarning( FText::FromString( 
					FString::Printf( TEXT( "Duplicate CardID found: %s" ), *card->CardID.ToString() )
				) );
			}
			seenIDs.Add( card->CardID );
		}
	}

	return result;
}
#endif
