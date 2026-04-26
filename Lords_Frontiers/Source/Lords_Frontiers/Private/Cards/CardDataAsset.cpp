#include "Cards/CardDataAsset.h"

#include "Cards/CardCondition.h"
#include "Cards/CardEffect.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

FLinearColor UCardDataAsset::GetRarityColor() const
{
	switch ( Rarity )
	{
	case ECardRarity::Common:
		return FLinearColor( 0.6f, 0.6f, 0.6f, 1.0f );

	case ECardRarity::Uncommon:
		return FLinearColor( 0.2f, 0.8f, 0.2f, 1.0f );

	case ECardRarity::Rare:
		return FLinearColor( 0.2f, 0.4f, 1.0f, 1.0f );

	case ECardRarity::Epic:
		return FLinearColor( 0.6f, 0.2f, 0.8f, 1.0f );

	case ECardRarity::Legendary:
		return FLinearColor( 1.0f, 0.6f, 0.0f, 1.0f );

	default:
		return FLinearColor::White;
	}
}

FText UCardDataAsset::BuildDescription() const
{
	if ( !Description.IsEmpty() )
	{
		return Description;
	}

	TArray<FString> parts;

	for ( const FCardEvent& event : Events )
	{
		for ( const TObjectPtr<UCardEffect>& effect : event.Effects )
		{
			if ( !effect )
			{
				continue;
			}
			FText text = effect->GetDisplayText();
			if ( !text.IsEmpty() )
			{
				parts.Add( text.ToString() );
			}
		}
	}

	return FText::FromString( FString::Join( parts, TEXT( "\n" ) ) );
}

FText UCardDataAsset::GetTargetDescription() const
{
	if ( Events.Num() == 0 )
	{
		return FText::GetEmpty();
	}

	if ( Events.Num() == 1 )
	{
		return Events[0].TargetFilter.GetTargetDescription();
	}

	TSet<FString> uniqueTargets;
	for ( const FCardEvent& event : Events )
	{
		uniqueTargets.Add( event.TargetFilter.GetTargetDescription().ToString() );
	}

	TArray<FString> asArray = uniqueTargets.Array();
	return FText::FromString( FString::Join( asArray, TEXT( ", " ) ) );
}

#if WITH_EDITOR
EDataValidationResult UCardDataAsset::IsDataValid( FDataValidationContext& context ) const
{
	EDataValidationResult result = Super::IsDataValid( context );

	if ( CardID.IsNone() )
	{
		context.AddError( FText::FromString( TEXT( "CardID must be set" ) ) );
		result = EDataValidationResult::Invalid;
	}

	if ( CardName.IsEmpty() )
	{
		context.AddError( FText::FromString( TEXT( "CardName must be set" ) ) );
		result = EDataValidationResult::Invalid;
	}

	if ( Events.Num() == 0 )
	{
		context.AddWarning( FText::FromString(
			TEXT( "Card has no events — it will have no effect when applied" ) ) );
	}

	for ( int32 i = 0; i < Events.Num(); ++i )
	{
		const FCardEvent& event = Events[i];

		if ( event.Effects.Num() == 0 )
		{
			context.AddWarning( FText::FromString(
				FString::Printf( TEXT( "Event %d has no effects" ), i ) ) );
		}

		for ( int32 j = 0; j < event.Effects.Num(); ++j )
		{
			if ( !event.Effects[j] )
			{
				context.AddWarning( FText::FromString(
					FString::Printf( TEXT( "Event %d, Effect %d is null" ), i, j ) ) );
			}
		}

		for ( int32 j = 0; j < event.Conditions.Num(); ++j )
		{
			if ( !event.Conditions[j] )
			{
				context.AddWarning( FText::FromString(
					FString::Printf( TEXT( "Event %d, Condition %d is null" ), i, j ) ) );
			}
		}

		if ( event.TargetFilter.BuildingType == EBuildingType::Specific &&
		     event.TargetFilter.SpecificBuildingClasses.Num() == 0 )
		{
			context.AddError( FText::FromString(
				FString::Printf( TEXT( "Event %d: Specific BuildingType selected but no classes listed" ), i ) ) );
			result = EDataValidationResult::Invalid;
		}
	}

	if ( BaseWeight <= 0.f )
	{
		context.AddError( FText::FromString( TEXT( "BaseWeight must be > 0" ) ) );
		result = EDataValidationResult::Invalid;
	}

	for ( const auto& pair : WeightMultipliers_Up )
	{
		if ( pair.Value <= 0.f )
		{
			context.AddWarning( FText::FromString( TEXT( "WeightMultipliers_Up has non-positive value" ) ) );
		}
	}
	for ( const auto& pair : WeightMultipliers_Down )
	{
		if ( pair.Value <= 0.f )
		{
			context.AddWarning( FText::FromString( TEXT( "WeightMultipliers_Down has non-positive value" ) ) );
		}
	}

	if ( ExcludedCards.Contains( const_cast<UCardDataAsset*>( this ) ) )
	{
		context.AddWarning( FText::FromString( TEXT( "Card excludes itself — redundant, will be filtered anyway" ) ) );
	}

	return result;
}
#endif
