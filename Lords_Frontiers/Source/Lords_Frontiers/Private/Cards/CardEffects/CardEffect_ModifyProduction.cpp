#include "Cards/CardEffects/CardEffect_ModifyProduction.h"

#include "Building/ResourceBuilding.h"

void UCardEffect_ModifyProduction::Apply_Implementation( const FCardEffectContext& context )
{
	ApplyDelta( context, Delta );
}

void UCardEffect_ModifyProduction::Revert_Implementation( const FCardEffectContext& context )
{
	ApplyDelta( context, -Delta );
}

void UCardEffect_ModifyProduction::ApplyDelta( const FCardEffectContext& context, int32 signedDelta ) const
{
	AResourceBuilding* resourceBuilding = Cast<AResourceBuilding>( context.Building.Get() );
	if ( !resourceBuilding || signedDelta == 0 )
	{
		return;
	}

	if ( ResourceTarget == EResourceTargetType::All )
	{
		resourceBuilding->ModifyProductionAll( signedDelta );
	}
	else
	{
		resourceBuilding->ModifyProduction( CardTypeHelpers::ToResourceType( ResourceTarget ), signedDelta );
	}
}

FText UCardEffect_ModifyProduction::GetDisplayText_Implementation() const
{
	const TCHAR* sign = Delta >= 0 ? TEXT( "+" ) : TEXT( "" );
	return FText::FromString(
		FString::Printf( TEXT( "%s%d %s production" ),
			sign, Delta, *CardTypeHelpers::GetResourceName( ResourceTarget ) ) );
}
