#include "Cards/CardEffects/CardEffect_ModifyMaintenance.h"

#include "Building/Building.h"

namespace
{
	void ApplyResourceDelta( FResourceProduction& production, EResourceTargetType target, int32 signedDelta )
	{
		if ( signedDelta == 0 )
		{
			return;
		}

		auto applyOne = [&]( EResourceType type )
		{
			switch ( type )
			{
			case EResourceType::Gold:
				production.Gold += signedDelta;
				break;
			case EResourceType::Food:
				production.Food += signedDelta;
				break;
			case EResourceType::Population:
				production.Population += signedDelta;
				break;
			default:
				break;
			}
		};

		if ( target == EResourceTargetType::All )
		{
			applyOne( EResourceType::Gold );
			applyOne( EResourceType::Food );
			applyOne( EResourceType::Population );
		}
		else
		{
			applyOne( CardTypeHelpers::ToResourceType( target ) );
		}
	}
}

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

void UCardEffect_ModifyMaintenance::PreviewBuildingTooltip_Implementation(
    const ABuilding* building, FEntityStats& InOutStats, FResourceProduction& InOutBuildingCost,
    FResourceProduction& InOutMaintenanceCost
) const
{
	if ( !building )
	{
		return;
	}

	ApplyResourceDelta( InOutMaintenanceCost, ResourceTarget, Delta );
}