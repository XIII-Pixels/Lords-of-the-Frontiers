#pragma once

#include "Cards/CardTypes.h"

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "CardPoolConfig.generated.h"

class UCardDataAsset;
class UCardRarityPoolConfig;

/**
 * FCardRarityWaveWeightOverride
 *
 * One-shot override of rarity tier weights for a specific wave. The multipliers
 * apply ONLY to the listed wave; the next wave returns to the base RarityWeight
 * defined on each UCardRarityPoolConfig. Rarities not present in RarityMultipliers
 * keep their base weight (multiplier = 1.0).
 *
 * Typical use: spike the chance of a Legendary card on a milestone wave.
 */
USTRUCT( BlueprintType )
struct LORDS_FRONTIERS_API FCardRarityWaveWeightOverride
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Override",
		meta = ( ClampMin = "0",
			ToolTip = "Wave number this override applies to. The multiplier is NOT carried into the next wave." ) )
	int32 WaveNumber = 0;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Override",
		meta = ( ToolTip = "Per-rarity multiplier applied to UCardRarityPoolConfig::RarityWeight on this wave. Missing rarities default to 1.0." ) )
	TMap<ECardRarity, float> RarityMultipliers;
};

UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UCardPoolConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Card Pool",
		meta = ( ToolTip = "One UCardRarityPoolConfig per rarity tier. The resolver picks a tier weighted by RarityWeight, then picks a card inside it weighted by BaseWeight + synergies." ) )
	TArray<TObjectPtr<UCardRarityPoolConfig>> RarityPools;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Card Pool",
		meta = ( ToolTip = "Per-wave overrides for rarity tier weights. Each entry applies its multipliers only on its WaveNumber; the next wave resets to base RarityWeight. Multiple entries with the same WaveNumber are multiplied together." ) )
	TArray<FCardRarityWaveWeightOverride> WaveRarityWeightOverrides;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Starting Cards" )
	TArray<TObjectPtr<UCardDataAsset>> StartingCards;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Selection", meta = ( ClampMin = "1", ClampMax = "10" ) )
	int32 CardsToOffer = 4;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Selection", meta = ( ClampMin = "1", ClampMax = "5" ) )
	int32 CardsToSelect = 2;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Selection",
		meta = ( ClampMin = "1", ClampMax = "20" ) )
	int32 MaxStacksForWeightInfluence = 3;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Selection",
		meta = ( ClampMin = "1",
			ToolTip = "Maximum number of cards of any single rarity that can appear in one offering." ) )
	int32 MaxCardsPerRarityInOffering = 2;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Reroll" )
	bool bAllowReroll = false;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Reroll",
		meta = ( EditCondition = "bAllowReroll" ) )
	EResourceTargetType RerollResource = EResourceTargetType::Gold;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Reroll",
		meta = ( ClampMin = "0", EditCondition = "bAllowReroll" ) )
	int32 RerollBaseCost = 50;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Reroll",
		meta = ( ClampMin = "0", EditCondition = "bAllowReroll",
			ToolTip = "Extra cost added per reroll already performed in the current selection." ) )
	int32 RerollCostIncrement = 25;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Reroll",
		meta = ( ClampMin = "0", EditCondition = "bAllowReroll",
			ToolTip = "Maximum number of rerolls allowed per selection. 0 means unlimited." ) )
	int32 MaxRerollsPerSelection = 0;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Reroll",
		meta = ( ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bAllowReroll",
			ToolTip = "Weight multiplier applied to cards that have already been shown during the current selection. Lower values make them very unlikely to appear again on a reroll. 1.0 disables the effect; 0.0 effectively excludes them. The list of seen cards is cleared once the player takes the reward." ) )
	float RerollSeenWeightMultiplier = 0.05f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Debug" )
	bool bDebugShowAllCards = false;

	UFUNCTION( BlueprintPure, Category = "Card Pool" )
	int32 GetPoolSize() const;

	UFUNCTION( BlueprintPure, Category = "Card Pool" )
	bool HasEnoughCards() const
	{
		return GetPoolSize() >= CardsToOffer;
	}

	UFUNCTION( BlueprintPure, Category = "Card Pool|Reroll" )
	int32 GetRerollCost( int32 rerollIndex ) const
	{
		return RerollBaseCost + FMath::Max( 0, rerollIndex ) * RerollCostIncrement;
	}

	/**
	 * Returns the combined multiplier to apply to a rarity tier's RarityWeight on the given wave.
	 * If multiple overrides target the same wave, their multipliers are combined multiplicatively.
	 * Rarities not mentioned by any matching override get 1.0.
	 */
	UFUNCTION( BlueprintPure, Category = "Card Pool|Wave Overrides" )
	float GetRarityWeightMultiplierForWave( int32 waveNumber, ECardRarity rarity ) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid( class FDataValidationContext& context ) const override;
#endif
};
