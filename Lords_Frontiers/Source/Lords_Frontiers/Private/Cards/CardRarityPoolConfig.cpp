#include "Cards/CardRarityPoolConfig.h"

#include "Cards/CardDataAsset.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#include "UObject/Class.h"
#endif

#if WITH_EDITOR
EDataValidationResult UCardRarityPoolConfig::IsDataValid( FDataValidationContext& context ) const
{
	EDataValidationResult result = Super::IsDataValid( context );

	if ( Cards.Num() == 0 )
	{
		context.AddError( FText::FromString( TEXT( "Rarity pool has no cards" ) ) );
		result = EDataValidationResult::Invalid;
	}

	const UEnum* rarityEnum = StaticEnum<ECardRarity>();
	const FString rarityName = rarityEnum ? rarityEnum->GetNameStringByValue( static_cast<int64>( Rarity ) ) : FString();

	int32 nullCount = 0;
	TSet<UCardDataAsset*> seen;

	for ( const TObjectPtr<UCardDataAsset>& card : Cards )
	{
		if ( !card )
		{
			++nullCount;
			continue;
		}

		if ( seen.Contains( card.Get() ) )
		{
			context.AddWarning( FText::FromString(
				FString::Printf( TEXT( "Duplicate card '%s' in rarity pool" ), *card->GetName() )
			) );
		}
		seen.Add( card.Get() );

		if ( card->Rarity != Rarity )
		{
			context.AddWarning( FText::FromString(
				FString::Printf(
					TEXT( "Card '%s' has rarity %s but is bucketed under %s" ),
					*card->GetName(),
					rarityEnum ? *rarityEnum->GetNameStringByValue( static_cast<int64>( card->Rarity ) ) : TEXT( "?" ),
					*rarityName )
			) );
		}
	}

	if ( nullCount > 0 )
	{
		context.AddWarning( FText::FromString(
			FString::Printf( TEXT( "Rarity pool contains %d null entries" ), nullCount )
		) );
	}

	return result;
}
#endif
