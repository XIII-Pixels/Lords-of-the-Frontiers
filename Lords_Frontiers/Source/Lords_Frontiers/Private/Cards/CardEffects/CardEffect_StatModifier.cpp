#include "Cards/CardEffects/CardEffect_StatModifier.h"

#include "Building/Building.h"
#include "Cards/CardEffects/StatReflectionHelpers.h"
#include "EntityStats.h"

#include "UObject/UnrealType.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

DEFINE_LOG_CATEGORY_STATIC( LogCardStatModifier, Log, All );

namespace
{
	const FName CardModifiableMeta( TEXT( "CardModifiable" ) );
}

void UCardEffect_StatModifier::Apply_Implementation( const FCardEffectContext& context )
{
	ApplyDelta( context, Delta );
}

void UCardEffect_StatModifier::Revert_Implementation( const FCardEffectContext& context )
{
	ApplyDelta( context, -Delta );
}

void UCardEffect_StatModifier::ApplyDelta( const FCardEffectContext& context, float signedDelta ) const
{
	ABuilding* building = context.Building.Get();
	if ( !building )
	{
		return;
	}

	FNumericProperty* prop = CardStatReflection::FindNumericProperty( StatName );
	if ( !prop )
	{
		UE_LOG(
		    LogCardStatModifier, Warning,
		    TEXT( "StatName '%s' not found on FEntityStats — skipping" ),
		    *StatName.ToString()
		);
		return;
	}

	FEntityStats& stats = building->Stats();
	void* addr = prop->ContainerPtrToValuePtr<void>( &stats );

	const double current = prop->GetFloatingPointPropertyValue( addr );
	prop->SetFloatingPointPropertyValue( addr, current + signedDelta );
}

FText UCardEffect_StatModifier::GetDisplayText_Implementation() const
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

	const TCHAR* sign = Delta >= 0.f ? TEXT( "+" ) : TEXT( "" );
	return FText::FromString(
		FString::Printf( TEXT( "%s%g %s" ), sign, Delta, *displayName ) );
}

TArray<FString> UCardEffect_StatModifier::GetModifiableStatNames()
{
	TArray<FString> result;

	UScriptStruct* entityStatsStruct = FEntityStats::StaticStruct();
	if ( !entityStatsStruct )
	{
		return result;
	}

	for ( TFieldIterator<FNumericProperty> it( entityStatsStruct ); it; ++it )
	{
#if WITH_EDITOR
		if ( !it->HasMetaData( CardModifiableMeta ) )
		{
			continue;
		}
#endif
		result.Add( it->GetName() );
	}

	return result;
}

#if WITH_EDITOR
EDataValidationResult UCardEffect_StatModifier::IsDataValid( FDataValidationContext& context ) const
{
	EDataValidationResult result = Super::IsDataValid( context );

	if ( StatName.IsNone() )
	{
		context.AddError( FText::FromString( TEXT( "StatName must be set" ) ) );
		return EDataValidationResult::Invalid;
	}

	FProperty* prop = FEntityStats::StaticStruct()->FindPropertyByName( StatName );
	if ( !prop )
	{
		context.AddError( FText::FromString(
			FString::Printf( TEXT( "StatName '%s' does not exist on FEntityStats" ),
				*StatName.ToString() )
		) );
		return EDataValidationResult::Invalid;
	}

	if ( !CastField<FNumericProperty>( prop ) )
	{
		context.AddError( FText::FromString(
			FString::Printf( TEXT( "StatName '%s' is not a numeric property" ),
				*StatName.ToString() )
		) );
		return EDataValidationResult::Invalid;
	}

	if ( !prop->HasMetaData( CardModifiableMeta ) )
	{
		context.AddWarning( FText::FromString(
			FString::Printf( TEXT( "StatName '%s' is not tagged CardModifiable" ),
				*StatName.ToString() )
		) );
	}

	if ( FMath::IsNearlyZero( Delta ) )
	{
		context.AddWarning( FText::FromString( TEXT( "Delta is zero — effect does nothing" ) ) );
	}

	return result;
}
#endif
