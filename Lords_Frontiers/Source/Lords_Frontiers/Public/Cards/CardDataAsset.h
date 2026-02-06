#pragma once

#include "Cards/CardTypes.h"

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "CardDataAsset.generated.h"

class ABuilding;

/**
 * UCardDataAsset
 *
 * Data Asset defining a single card's properties.
 * Create in Editor: Right-click -> Miscellaneous -> Data Asset -> CardDataAsset
 */
UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UCardDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UCardDataAsset();

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Identity" )
	FName CardID = NAME_None;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Display" )
	FText CardName;

	/**
	 * Description template. Use {0}, {1} for modifier values.
	 * Example: "Increases damage by {0}"
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Display", meta = ( MultiLine = true ) )
	FText Description;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Display" )
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Display" )
	ECardRarity Rarity = ECardRarity::Common;

	/**
	 * Which buildings this card affects.
	 * When TargetFilter == Any AND card has resource modifiers:
	 *   modifiers are applied as global economy bonuses.
	 * Otherwise: modifiers are applied directly to matching buildings.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Targeting" )
	FBuildingTargetFilter TargetFilter;

	/**
	 * Stat modifiers applied by this card.
	 *
	 * Building Stats (MaxHealth, AttackDamage, etc.):
	 *   - Applied to buildings matching TargetFilter
	 *   - Modifies FEntityStats
	 *
	 * Resource Modifiers (MaintenanceCost, Production):
	 *   - TargetFilter == Any  → global economy bonus (FEconomyBonuses)
	 *   - TargetFilter != Any  → applied directly to matching buildings
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Modifiers" )
	TArray<FCardStatModifier> Modifiers;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Selection", meta = ( ClampMin = "0.1" ) )
	float SelectionWeight = 100.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Selection" )
	bool bCanStack = true;

	UFUNCTION( BlueprintPure, Category = "Card" )
	bool AppliesToBuilding( const ABuilding* building ) const;

	UFUNCTION( BlueprintPure, Category = "Card" )
	FText GetFormattedDescription() const;

	UFUNCTION( BlueprintPure, Category = "Card" )
	FText GetTargetDescription() const;

	UFUNCTION( BlueprintPure, Category = "Card" )
	FLinearColor GetRarityColor() const;

	UFUNCTION( BlueprintPure, Category = "Card" )
	bool HasValidModifiers() const;

	/** Returns true if this card has building stat modifiers (FEntityStats). */
	UFUNCTION( BlueprintPure, Category = "Card" )
	bool HasBuildingModifiers() const;

	/** Returns true if this card has resource modifiers (MaintenanceCost, Production). */
	UFUNCTION( BlueprintPure, Category = "Card" )
	bool HasResourceModifiers() const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid( class FDataValidationContext& context ) const override;
#endif
};
