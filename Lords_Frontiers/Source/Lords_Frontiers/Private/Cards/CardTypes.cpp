#include "Cards/CardTypes.h"

#include "Building/AdditiveBuilding.h"
#include "Building/Building.h"
#include "Building/DefensiveBuilding.h"
#include "Building/MainBase.h"
#include "Building/ResourceBuilding.h"

FText FBuildingTargetFilter::GetTargetDescription() const
{
	switch ( BuildingType )
	{
	case EBuildingType::Any:
		return FText::FromString( TEXT( "All Buildings" ) );

	case EBuildingType::Defensive:
		return FText::FromString( TEXT( "Defensive Buildings" ) );

	case EBuildingType::Resource:
		return FText::FromString( TEXT( "Resource Buildings" ) );

	case EBuildingType::Additive:
		return FText::FromString( TEXT( "Walls & Utilities" ) );

	case EBuildingType::MainBase:
		return FText::FromString( TEXT( "Main Base" ) );

	case EBuildingType::Specific:
	{
		if ( SpecificBuildingClasses.Num() == 0 )
		{
			return FText::FromString( TEXT( "No Classes Selected" ) );
		}
		else if ( SpecificBuildingClasses.Num() == 1 && SpecificBuildingClasses[0] )
		{
			return FText::FromString( SpecificBuildingClasses[0]->GetName() );
		}
		else
		{
			return FText::FromString(
				FString::Printf( TEXT( "%d Specific Classes" ), SpecificBuildingClasses.Num() )
			);
		}
	}

	default:
		break;
	}

	return FText::FromString( TEXT( "Unknown" ) );
}

bool FBuildingTargetFilter::MatchesBuilding( const AActor* building ) const
{
	if ( !building )
	{
		return false;
	}

	switch ( BuildingType )
	{
	case EBuildingType::Any:
		return true;

	case EBuildingType::Defensive:
		return building->IsA<ADefensiveBuilding>();

	case EBuildingType::Resource:
		return building->IsA<AResourceBuilding>();

	case EBuildingType::Additive:
		return building->IsA<AAdditiveBuilding>();

	case EBuildingType::MainBase:
		return building->IsA<AMainBase>();

	case EBuildingType::Specific:
	{
		for ( const TSubclassOf<AActor>& buildingClass : SpecificBuildingClasses )
		{
			if ( buildingClass && building->IsA( buildingClass ) )
			{
				return true;
			}
		}
		return false;
	}

	default:
		return false;
	}
}

bool FCardEvent::MatchesBuilding( const ABuilding* building ) const
{
	if ( !building )
	{
		return false;
	}

	if ( !TargetFilter.MatchesBuilding( building ) )
	{
		return false;
	}

	const bool bRuined = building->IsRuined();

	switch ( StateFilter )
	{
	case ETargetStateFilter::Alive: return !bRuined;
	case ETargetStateFilter::Dead:  return bRuined;
	case ETargetStateFilter::Any:   return true;
	default:                        return !bRuined;
	}
}

void FEconomyBonuses::AddProductionBonus( EResourceTargetType target, int32 delta )
{
	if ( target == EResourceTargetType::All || target == EResourceTargetType::Gold )
	{
		GoldProductionBonus += delta;
	}
	if ( target == EResourceTargetType::All || target == EResourceTargetType::Food )
	{
		FoodProductionBonus += delta;
	}
	if ( target == EResourceTargetType::All || target == EResourceTargetType::Population )
	{
		PopulationProductionBonus += delta;
	}
	if ( target == EResourceTargetType::All || target == EResourceTargetType::Progress )
	{
		ProgressProductionBonus += delta;
	}
}

void FEconomyBonuses::AddMaintenanceReduction( EResourceTargetType target, int32 delta )
{
	if ( target == EResourceTargetType::All || target == EResourceTargetType::Gold )
	{
		GoldMaintenanceReduction += delta;
	}
	if ( target == EResourceTargetType::All || target == EResourceTargetType::Food )
	{
		FoodMaintenanceReduction += delta;
	}
	if ( target == EResourceTargetType::All || target == EResourceTargetType::Population )
	{
		PopulationMaintenanceReduction += delta;
	}
	if ( target == EResourceTargetType::All || target == EResourceTargetType::Progress )
	{
		ProgressMaintenanceReduction += delta;
	}
}

int32 FEconomyBonuses::GetProductionBonus( EResourceTargetType target ) const
{
	switch ( target )
	{
	case EResourceTargetType::Gold:       return GoldProductionBonus;
	case EResourceTargetType::Food:       return FoodProductionBonus;
	case EResourceTargetType::Population: return PopulationProductionBonus;
	case EResourceTargetType::Progress:   return ProgressProductionBonus;
	case EResourceTargetType::All:
		return GoldProductionBonus + FoodProductionBonus
		     + PopulationProductionBonus + ProgressProductionBonus;
	default:
		return 0;
	}
}

int32 FEconomyBonuses::GetMaintenanceReduction( EResourceTargetType target ) const
{
	switch ( target )
	{
	case EResourceTargetType::Gold:       return GoldMaintenanceReduction;
	case EResourceTargetType::Food:       return FoodMaintenanceReduction;
	case EResourceTargetType::Population: return PopulationMaintenanceReduction;
	case EResourceTargetType::Progress:   return ProgressMaintenanceReduction;
	case EResourceTargetType::All:
		return GoldMaintenanceReduction + FoodMaintenanceReduction
		     + PopulationMaintenanceReduction + ProgressMaintenanceReduction;
	default:
		return 0;
	}
}
