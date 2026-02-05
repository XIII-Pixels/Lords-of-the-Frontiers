#include "Cards/CardTypes.h"

#include "Building/Building.h"
#include "Building/DefensiveBuilding.h"
#include "Building/ResourceBuilding.h"
#include "Building/AdditiveBuilding.h"
#include "Building/MainBase.h"

FText FCardStatModifier::GetDisplayText() const
{
	if ( !IsValid() )
	{
		return FText::GetEmpty();
	}

	const FString resourceName = CardTypeHelpers::GetResourceName( ResourceTarget );
	FString statName;

	switch ( Stat )
	{
	case EBuildingStat::MaxHealth:
		statName = TEXT( "Max Health" );
		break;

	case EBuildingStat::AttackDamage:
		statName = TEXT( "Attack Damage" );
		break;

	case EBuildingStat::AttackRange:
		statName = TEXT( "Attack Range" );
		break;

	case EBuildingStat::AttackCooldown:
		statName = TEXT( "Attack Speed" );
		break;

	case EBuildingStat::MaxSpeed:
		statName = TEXT( "Speed" );
		break;

	case EBuildingStat::MaintenanceCost:
	{
		statName = FString::Printf( TEXT( "%s Maintenance Cost" ), *resourceName );
		const FString sign = FlatValue >= 0 ? TEXT( "+" ) : TEXT( "" );
		return FText::FromString(
			FString::Printf( TEXT( "%s%d %s" ), *sign, FlatValue, *statName )
		);
	}

	case EBuildingStat::BuildingProduction:
	{
		statName = FString::Printf( TEXT( "%s Production" ), *resourceName );
		break;
	}

	default:
		statName = TEXT( "Unknown" );
		break;
	}

	const FString sign = FlatValue >= 0 ? TEXT( "+" ) : TEXT( "" );
	return FText::FromString( FString::Printf( TEXT( "%s%d %s" ), *sign, FlatValue, *statName ) );
}

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

void FEconomyBonuses::ApplyModifier( const FCardStatModifier& modifier )
{
	if ( !modifier.IsResourceModifier() )
	{
		return;
	}

	auto applyToTargets = [&]( int32& gold, int32& food, int32& population, int32& progress, int32 value )
	{
		const EResourceTargetType target = modifier.ResourceTarget;

		if ( target == EResourceTargetType::All || target == EResourceTargetType::Gold )
		{
			gold += value;
		}
		if ( target == EResourceTargetType::All || target == EResourceTargetType::Food )
		{
			food += value;
		}
		if ( target == EResourceTargetType::All || target == EResourceTargetType::Population )
		{
			population += value;
		}
		if ( target == EResourceTargetType::All || target == EResourceTargetType::Progress )
		{
			progress += value;
		}
	};

	switch ( modifier.Stat )
	{
	case EBuildingStat::BuildingProduction:
		applyToTargets(
			GoldProductionBonus, FoodProductionBonus,
			PopulationProductionBonus, ProgressProductionBonus,
			modifier.FlatValue
		);
		break;

	case EBuildingStat::MaintenanceCost:
		applyToTargets(
			GoldMaintenanceReduction, FoodMaintenanceReduction,
			PopulationMaintenanceReduction, ProgressMaintenanceReduction,
			-modifier.FlatValue
		);
		break;

	default:
		break;
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
