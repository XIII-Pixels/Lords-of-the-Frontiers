#include "Cards/CardDataAsset.h"

#include "Building/Building.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

UCardDataAsset::UCardDataAsset()
{
	SelectionWeight = 100.0f;
}

bool UCardDataAsset::AppliesToBuilding( const ABuilding* building ) const
{
	return TargetFilter.MatchesBuilding( building );
}

FText UCardDataAsset::GetFormattedDescription() const
{
	FString result = Description.ToString();

	for ( int32 i = 0; i < Modifiers.Num(); ++i )
	{
		FString placeholder = FString::Printf( TEXT( "{%d}" ), i );
		FString value = FString::Printf( TEXT( "%d" ), FMath::Abs( Modifiers[i].FlatValue ) );
		result = result.Replace( *placeholder, *value );
	}

	return FText::FromString( result );
}

FText UCardDataAsset::GetTargetDescription() const
{
	return TargetFilter.GetTargetDescription();
}

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

bool UCardDataAsset::HasValidModifiers() const
{
	for ( const FCardStatModifier& mod : Modifiers )
	{
		if ( mod.IsValid() )
		{
			return true;
		}
	}
	return false;
}

bool UCardDataAsset::HasBuildingModifiers() const
{
	for ( const FCardStatModifier& mod : Modifiers )
	{
		if ( mod.IsBuildingStatModifier() )
		{
			return true;
		}
	}
	return false;
}

bool UCardDataAsset::HasResourceModifiers() const
{
	for ( const FCardStatModifier& mod : Modifiers )
	{
		if ( mod.IsResourceModifier() )
		{
			return true;
		}
	}
	return false;
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

	if ( Modifiers.Num() == 0 )
	{
		context.AddWarning( FText::FromString( TEXT( "Card has no modifiers" ) ) );
	}

	if ( TargetFilter.BuildingType == EBuildingType::Specific )
	{
		if ( TargetFilter.SpecificBuildingClasses.Num() == 0 )
		{
			context.AddError( FText::FromString(
				TEXT( "Specific BuildingType selected but no classes specified" ) )
			);
			result = EDataValidationResult::Invalid;
		}

		for ( int32 i = 0; i < TargetFilter.SpecificBuildingClasses.Num(); ++i )
		{
			if ( !TargetFilter.SpecificBuildingClasses[i] )
			{
				context.AddWarning( FText::FromString(
					FString::Printf( TEXT( "SpecificBuildingClasses[%d] is null" ), i )
				) );
			}
		}
	}

	for ( int32 i = 0; i < Modifiers.Num(); ++i )
	{
		const FCardStatModifier& mod = Modifiers[i];

		if ( !mod.IsValid() )
		{
			context.AddWarning( FText::FromString(
				FString::Printf( TEXT( "Modifier %d is invalid (None stat or zero value)" ), i )
			) );
		}

		if ( mod.Stat == EBuildingStat::BuildingProduction &&
		     TargetFilter.BuildingType != EBuildingType::Resource &&
		     TargetFilter.BuildingType != EBuildingType::Any &&
		     TargetFilter.BuildingType != EBuildingType::Specific )
		{
			context.AddWarning( FText::FromString(
				FString::Printf(
					TEXT( "Modifier %d is BuildingProduction but TargetFilter is not Resource — "
					      "will only affect AResourceBuilding subclasses" ), i )
			) );
		}
	}

	if ( SelectionWeight <= 0.0f )
	{
		context.AddError( FText::FromString( TEXT( "SelectionWeight must be > 0" ) ) );
		result = EDataValidationResult::Invalid;
	}

	return result;
}
#endif
