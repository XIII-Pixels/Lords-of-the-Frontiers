#include "Cards/CardEffects/CardEffect_ModifyMaintenance.h"

#include "Building/Building.h"

void UCardEffect_ModifyMaintenance::Apply_Implementation( const FCardEffectContext& context )
{
	ApplyDelta( context, Delta );
}

void UCardEffect_ModifyMaintenance::Revert_Implementation( const FCardEffectContext& context )
{
	ApplyDelta( context, -Delta );
}

void UCardEffect_ModifyMaintenance::ApplyDelta( const FCardEffectContext& context, int32 signedDelta ) const
{
	ABuilding* building = context.Building.Get();
	if ( !building || signedDelta == 0 )
	{
		return;
	}

	if ( ResourceTarget == EResourceTargetType::All )
	{
		building->ModifyMaintenanceCostAll( signedDelta );
	}
	else
	{
		building->ModifyMaintenanceCost( CardTypeHelpers::ToResourceType( ResourceTarget ), signedDelta );
	}
}

FText UCardEffect_ModifyMaintenance::GetDisplayText_Implementation() const
{
	const TCHAR* sign = Delta >= 0 ? TEXT( "+" ) : TEXT( "" );
	return FText::FromString(
		FString::Printf( TEXT( "%s%d %s maintenance" ),
			sign, Delta, *CardTypeHelpers::GetResourceName( ResourceTarget ) ) );
}
