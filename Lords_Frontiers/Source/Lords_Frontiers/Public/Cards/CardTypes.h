#pragma once

#include "CoreMinimal.h"
#include "Lords_Frontiers/Public/Resources/GameResource.h"

#include "CardTypes.generated.h"

class UCardDataAsset;
class UCardCondition;
class UCardEffect;
class UCardEffectHostComponent;
class UCardSubsystem;
class ABuilding;

UENUM( BlueprintType )
enum class ECardCategory : uint8
{
	None		UMETA( DisplayName = "None" ),
	Offensive	UMETA( DisplayName = "Offensive" ),
	Defensive	UMETA( DisplayName = "Defensive" ),
	Economy		UMETA( DisplayName = "Economy" ),
	Utility		UMETA( DisplayName = "Utility" ),
	Special		UMETA( DisplayName = "Special" ),
};

UENUM( BlueprintType )
enum class EResourceTargetType : uint8
{
	All			UMETA( DisplayName = "All Resources" ),
	Gold		UMETA( DisplayName = "Gold" ),
	Food		UMETA( DisplayName = "Food" ),
	Population	UMETA( DisplayName = "Population" ),
	Progress	UMETA( DisplayName = "Progress" ),
};

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

UENUM( BlueprintType )
enum class EDefensiveTowerType : uint8
{
	Generic		UMETA( DisplayName = "Generic" ),
	Speed		UMETA( DisplayName = "Speed Tower" ),
	Sniper		UMETA( DisplayName = "Sniper Tower" ),
	Magic		UMETA( DisplayName = "Magic Tower" ),
	Mortar		UMETA( DisplayName = "Mortar Tower" ),
};

UENUM( BlueprintType )
enum class ETargetStateFilter : uint8
{
	Alive	UMETA( DisplayName = "Alive Only" ),
	Dead	UMETA( DisplayName = "Destroyed Only" ),
	Any		UMETA( DisplayName = "Any State" ),
};

UENUM( BlueprintType )
enum class ECardStatusKind : uint8
{
	Burn		UMETA( DisplayName = "Burn (Fire DOT)" ),
	Slow		UMETA( DisplayName = "Slow (Movement)" ),
	AttackSlow	UMETA( DisplayName = "Slow (Attack Speed)" ),
	AnySlow		UMETA( DisplayName = "Slow (Any)" ),
};

UENUM( BlueprintType )
enum class ECardTriggerReason : uint8
{
	Apply				UMETA( DisplayName = "Apply" ),
	Revert				UMETA( DisplayName = "Revert" ),
	BeforeAttackFire	UMETA( DisplayName = "Before Attack Fire" ),
	AttackFired			UMETA( DisplayName = "Attack Fired" ),
	TargetChanged		UMETA( DisplayName = "Target Changed" ),
	HitLanded			UMETA( DisplayName = "Hit Landed" ),
	KillLanded			UMETA( DisplayName = "Kill Landed" ),
	Missed				UMETA( DisplayName = "Missed" ),
	Landed				UMETA( DisplayName = "Projectile Landed (ground impact)" ),
	Damaged				UMETA( DisplayName = "Damaged" ),
	Ruined				UMETA( DisplayName = "Ruined" ),
	AuraTick			UMETA( DisplayName = "Aura Tick" ),
};

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

	inline TArray<EResourceType> GetAllResourceTypes()
	{
		return {
			EResourceType::Gold,
			EResourceType::Food,
			EResourceType::Population,
			EResourceType::Progress
		};
	}

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

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Filter",
		meta = ( EditCondition = "BuildingType == EBuildingType::Defensive", EditConditionHides ) )
	TSet<EDefensiveTowerType> DefensiveTowerTypes;

	FText GetTargetDescription() const;

	bool MatchesBuilding( const AActor* building ) const;
};

USTRUCT( BlueprintType )
struct LORDS_FRONTIERS_API FCardEffectContext
{
	GENERATED_BODY()

	UPROPERTY( BlueprintReadOnly, Category = "Context" )
	TWeakObjectPtr<UCardDataAsset> SourceCard;

	UPROPERTY( BlueprintReadOnly, Category = "Context" )
	TWeakObjectPtr<ABuilding> Building;

	UPROPERTY( BlueprintReadOnly, Category = "Context" )
	TWeakObjectPtr<AActor> EventInstigator;

	UPROPERTY( BlueprintReadOnly, Category = "Context" )
	TWeakObjectPtr<UCardEffectHostComponent> EffectHost;

	UPROPERTY( BlueprintReadOnly, Category = "Context" )
	TWeakObjectPtr<UCardSubsystem> Subsystem;

	UPROPERTY( BlueprintReadOnly, Category = "Context" )
	int32 EventIndex = INDEX_NONE;

	UPROPERTY( BlueprintReadOnly, Category = "Context" )
	int32 WaveNumber = 0;

	UPROPERTY( BlueprintReadOnly, Category = "Context" )
	int32 StackCount = 1;

	UPROPERTY( BlueprintReadOnly, Category = "Context" )
	int32 ActionMagnitude = 0;

	UPROPERTY( BlueprintReadOnly, Category = "Context" )
	ECardTriggerReason TriggerReason = ECardTriggerReason::Apply;

	UPROPERTY( BlueprintReadOnly, Category = "Context" )
	FVector EventLocation = FVector::ZeroVector;

	UPROPERTY( BlueprintReadOnly, Category = "Context" )
	bool bHasEventLocation = false;
};

USTRUCT( BlueprintType )
struct LORDS_FRONTIERS_API FCardEvent
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Event" )
	FBuildingTargetFilter TargetFilter;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Event" )
	ETargetStateFilter StateFilter = ETargetStateFilter::Alive;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Event",
		meta = ( AllowAbstract = "false" ) )
	TArray<TObjectPtr<UCardCondition>> Conditions;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Event",
		meta = ( AllowAbstract = "false" ) )
	TArray<TObjectPtr<UCardEffect>> Effects;

	bool MatchesBuilding( const ABuilding* building ) const;
};

USTRUCT( BlueprintType )
struct LORDS_FRONTIERS_API FEconomyBonuses
{
	GENERATED_BODY()

	UPROPERTY( BlueprintReadOnly, Category = "Production" )
	int32 GoldProductionBonus = 0;

	UPROPERTY( BlueprintReadOnly, Category = "Production" )
	int32 FoodProductionBonus = 0;

	UPROPERTY( BlueprintReadOnly, Category = "Production" )
	int32 PopulationProductionBonus = 0;

	UPROPERTY( BlueprintReadOnly, Category = "Production" )
	int32 ProgressProductionBonus = 0;

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

	void AddProductionBonus( EResourceTargetType target, int32 delta );
	void AddMaintenanceReduction( EResourceTargetType target, int32 delta );
	int32 GetProductionBonus( EResourceTargetType target ) const;
	int32 GetMaintenanceReduction( EResourceTargetType target ) const;
};

USTRUCT( BlueprintType )
struct LORDS_FRONTIERS_API FAppliedCardBonus
{
	GENERATED_BODY()

	UPROPERTY( BlueprintReadOnly, Category = "Bonus" )
	TObjectPtr<UCardDataAsset> SourceCard = nullptr;

	UPROPERTY( BlueprintReadOnly, Category = "Bonus" )
	TObjectPtr<UCardEffect> Effect = nullptr;

	UPROPERTY( BlueprintReadOnly, Category = "Bonus" )
	int32 WaveApplied = 0;

	FAppliedCardBonus() = default;

	FAppliedCardBonus( UCardDataAsset* card, UCardEffect* effect, int32 wave )
		: SourceCard( card )
		, Effect( effect )
		, WaveApplied( wave )
	{
	}
};

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
