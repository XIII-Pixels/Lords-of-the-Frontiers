#pragma once

#include "CoreMinimal.h"
#include "Lords_Frontiers/Public/Resources/GameResource.h"

#include "CardTypes.generated.h"

class UCardDataAsset;

/**
 * EBuildingStat
 *
 * Stats that can be modified by cards.
 * Three categories:
 *   1. Building Stats — modify FEntityStats on matching buildings
 *   2. Global Economy — modify FEconomyBonuses (applied by EconomyComponent)
 *   3. Direct Building — modify building properties (MaintenanceCost, Production)
 */
UENUM( BlueprintType )
enum class EBuildingStat : uint8
{
	None						UMETA( DisplayName = "None" ),

	// === Building Stats (FEntityStats) ===
	MaxHealth					UMETA( DisplayName = "Max Health" ),
	AttackDamage				UMETA( DisplayName = "Attack Damage" ),
	AttackRange					UMETA( DisplayName = "Attack Range" ),
	AttackCooldown				UMETA( DisplayName = "Attack Cooldown" ),
	MaxSpeed					UMETA( DisplayName = "Max Speed (Suddenly there will be units:) )" ),

	// === Resource Modifiers ===
	// Routing depends on the card's TargetFilter:
	//   TargetFilter == Any  → applied as global economy bonus (FEconomyBonuses)
	//   TargetFilter != Any  → applied directly to matching buildings

	/** Modifies maintenance cost. Negative = cheaper. */
	MaintenanceCost				UMETA( DisplayName = "Maintenance Cost" ),

	/** Modifies resource production amount. */
	BuildingProduction			UMETA( DisplayName = "Production" ),
};

/**
 * EResourceTargetType
 *
 * Which resource type the economy/direct modifier affects.
 */
UENUM( BlueprintType )
enum class EResourceTargetType : uint8
{
	All			UMETA( DisplayName = "All Resources" ),
	Gold		UMETA( DisplayName = "Gold" ),
	Food		UMETA( DisplayName = "Food" ),
	Population	UMETA( DisplayName = "Population" ),
	Progress	UMETA( DisplayName = "Progress" ),
};

/**
 * EBuildingType
 *
 * Building categories for card targeting.
 */
UENUM( BlueprintType )
enum class EBuildingType : uint8
{
	Any				UMETA( DisplayName = "Any Building" ),
	Defensive		UMETA( DisplayName = "Defensive (Towers)" ),
	Resource		UMETA( DisplayName = "Resource Generators" ),
	Additive		UMETA( DisplayName = "Additive (Walls, Utilities)" ),
	MainBase		UMETA( DisplayName = "Main Base" ),
	Specific		UMETA( DisplayName = "Specific Classes" ),
};

/**
 * ECardRarity
 */
UENUM( BlueprintType )
enum class ECardRarity : uint8
{
	Common		UMETA( DisplayName = "Common" ),
	Uncommon	UMETA( DisplayName = "Uncommon" ),
	Rare		UMETA( DisplayName = "Rare" ),
	Epic		UMETA( DisplayName = "Epic" ),
	Legendary	UMETA( DisplayName = "Legendary" ),
};

namespace CardTypeHelpers
{
	/** Converts EResourceTargetType to EResourceType. Returns None for All. */
	inline EResourceType ToResourceType( EResourceTargetType target )
	{
		switch ( target )
		{
		case EResourceTargetType::Gold:       return EResourceType::Gold;
		case EResourceTargetType::Food:       return EResourceType::Food;
		case EResourceTargetType::Population: return EResourceType::Population;
		case EResourceTargetType::Progress:   return EResourceType::Progress;
		default:                              return EResourceType::None;
		}
	}

	/** Converts EResourceType to EResourceTargetType. Returns All for None/Max. */
	inline EResourceTargetType ToResourceTargetType( EResourceType type )
	{
		switch ( type )
		{
		case EResourceType::Gold:       return EResourceTargetType::Gold;
		case EResourceType::Food:       return EResourceTargetType::Food;
		case EResourceType::Population: return EResourceTargetType::Population;
		case EResourceType::Progress:   return EResourceTargetType::Progress;
		default:                        return EResourceTargetType::All;
		}
	}

	/** Returns array of all concrete resource types (excludes None/Max). */
	inline TArray<EResourceType> GetAllResourceTypes()
	{
		return {
			EResourceType::Gold,
			EResourceType::Food,
			EResourceType::Population,
			EResourceType::Progress
		};
	}

	/** Returns resource display name. */
	inline FString GetResourceName( EResourceTargetType target )
	{
		switch ( target )
		{
		case EResourceTargetType::Gold:       return TEXT( "Gold" );
		case EResourceTargetType::Food:       return TEXT( "Food" );
		case EResourceTargetType::Population: return TEXT( "Population" );
		case EResourceTargetType::Progress:   return TEXT( "Progress" );
		case EResourceTargetType::All:        return TEXT( "All Resources" );
		default:                              return TEXT( "Unknown" );
		}
	}
}

/**
 * FCardStatModifier
 *
 * Single stat modification applied by a card.
 * Uses flat values (not percentages).
 */
USTRUCT( BlueprintType )
struct LORDS_FRONTIERS_API FCardStatModifier
{
	GENERATED_BODY()

	/** Which stat to modify */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Modifier" )
	EBuildingStat Stat = EBuildingStat::None;

	/** Flat value to add (can be negative) */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Modifier" )
	int32 FlatValue = 0;

	/**
	 * Which resource type to affect.
	 * Shown for: MaintenanceCost, BuildingProduction.
	 */
UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Modifier",
    meta = ( EditCondition = "Stat == EBuildingStat::MaintenanceCost || Stat == EBuildingStat::BuildingProduction",
        EditConditionHides ) )
	EResourceTargetType ResourceTarget = EResourceTargetType::All;

	FCardStatModifier() = default;

	FCardStatModifier( EBuildingStat stat, int32 value )
		: Stat( stat )
		, FlatValue( value )
	{
	}

	FCardStatModifier( EBuildingStat stat, int32 value, EResourceTargetType resourceTarget )
		: Stat( stat )
		, FlatValue( value )
		, ResourceTarget( resourceTarget )
	{
	}

	bool IsValid() const
	{
		return Stat != EBuildingStat::None && FlatValue != 0;
	}

	/** Returns true if this is a resource modifier (MaintenanceCost or BuildingProduction) */
	bool IsResourceModifier() const
	{
		return Stat == EBuildingStat::MaintenanceCost ||
		       Stat == EBuildingStat::BuildingProduction;
	}

	/** Returns true if this modifies FEntityStats (MaxHealth, AttackDamage, etc.) */
	bool IsBuildingStatModifier() const
	{
		return IsValid() && !IsResourceModifier();
	}

	/** Returns display text like "+10 Attack Damage" or "-3 Gold Maintenance" */
	FText GetDisplayText() const;
};

/**
 * FBuildingTargetFilter
 */
USTRUCT( BlueprintType )
struct LORDS_FRONTIERS_API FBuildingTargetFilter
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Filter" )
	EBuildingType BuildingType = EBuildingType::Any;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Filter",
		meta = ( EditCondition = "BuildingType == EBuildingType::Specific", EditConditionHides,
			AllowAbstract = "false" ) )
	TArray<TSubclassOf<AActor>> SpecificBuildingClasses;

	FText GetTargetDescription() const;

	bool MatchesBuilding( const AActor* building ) const;
};

/**
 * FEconomyBonuses
 *
 * Aggregated economy bonuses from all applied cards.
 * Used by EconomyComponent for global resource calculations.
 */
USTRUCT( BlueprintType )
struct LORDS_FRONTIERS_API FEconomyBonuses
{
	GENERATED_BODY()

	// === Production Bonuses ===
	UPROPERTY( BlueprintReadOnly, Category = "Production" )
	int32 GoldProductionBonus = 0;

	UPROPERTY( BlueprintReadOnly, Category = "Production" )
	int32 FoodProductionBonus = 0;

	UPROPERTY( BlueprintReadOnly, Category = "Production" )
	int32 PopulationProductionBonus = 0;

	UPROPERTY( BlueprintReadOnly, Category = "Production" )
	int32 ProgressProductionBonus = 0;

	// === Maintenance Cost Reduction ===
	UPROPERTY( BlueprintReadOnly, Category = "Costs" )
	int32 GoldMaintenanceReduction = 0;

	UPROPERTY( BlueprintReadOnly, Category = "Costs" )
	int32 FoodMaintenanceReduction = 0;

	UPROPERTY( BlueprintReadOnly, Category = "Costs" )
	int32 PopulationMaintenanceReduction = 0;

	UPROPERTY( BlueprintReadOnly, Category = "Costs" )
	int32 ProgressMaintenanceReduction = 0;

	void Reset()
	{
		GoldProductionBonus = 0;
		FoodProductionBonus = 0;
		PopulationProductionBonus = 0;
		ProgressProductionBonus = 0;
		GoldMaintenanceReduction = 0;
		FoodMaintenanceReduction = 0;
		PopulationMaintenanceReduction = 0;
		ProgressMaintenanceReduction = 0;
	}

	/**
	 * Applies a resource modifier as a global economy bonus.
	 * MaintenanceCost: negative FlatValue → positive reduction
	 * BuildingProduction: FlatValue added directly as production bonus
	 */
	void ApplyModifier( const FCardStatModifier& modifier );

	/** Returns production bonus for a specific target type */
	int32 GetProductionBonus( EResourceTargetType target ) const;

	/** Returns maintenance reduction for a specific target type */
	int32 GetMaintenanceReduction( EResourceTargetType target ) const;
};

/**
 * FAppliedCardBonus
 */
USTRUCT( BlueprintType )
struct LORDS_FRONTIERS_API FAppliedCardBonus
{
	GENERATED_BODY()

	UPROPERTY( BlueprintReadOnly, Category = "Bonus" )
	TObjectPtr<UCardDataAsset> SourceCard = nullptr;

	UPROPERTY( BlueprintReadOnly, Category = "Bonus" )
	FCardStatModifier Modifier;

	UPROPERTY( BlueprintReadOnly, Category = "Bonus" )
	int32 WaveApplied = 0;

	FAppliedCardBonus() = default;

	FAppliedCardBonus( UCardDataAsset* card, const FCardStatModifier& mod, int32 wave )
		: SourceCard( card )
		, Modifier( mod )
		, WaveApplied( wave )
	{
	}
};

/**
 * FCardChoice
 */
USTRUCT( BlueprintType )
struct LORDS_FRONTIERS_API FCardChoice
{
	GENERATED_BODY()

	UPROPERTY( BlueprintReadOnly, Category = "Choice" )
	TArray<TObjectPtr<UCardDataAsset>> AvailableCards;

	UPROPERTY( BlueprintReadOnly, Category = "Choice" )
	int32 CardsToSelect = 1;

	UPROPERTY( BlueprintReadOnly, Category = "Choice" )
	int32 WaveNumber = 0;
};

/**
 * FAppliedCardRecord
 */
USTRUCT( BlueprintType )
struct LORDS_FRONTIERS_API FAppliedCardRecord
{
	GENERATED_BODY()

	UPROPERTY( BlueprintReadOnly, Category = "Record" )
	TObjectPtr<UCardDataAsset> Card = nullptr;

	UPROPERTY( BlueprintReadOnly, Category = "Record" )
	int32 WaveSelected = 0;

	UPROPERTY( BlueprintReadOnly, Category = "Record" )
	int32 StackCount = 1;

	FAppliedCardRecord() = default;

	FAppliedCardRecord( UCardDataAsset* card, int32 wave )
		: Card( card )
		, WaveSelected( wave )
		, StackCount( 1 )
	{
	}
};
