#pragma once

#include "Cards/CardTypes.h"

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "CardPoolConfig.generated.h"

class UCardDataAsset;
class UCardRarityPoolConfig;

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
			ToolTip = "Множитель веса ОТДЕЛЬНЫХ карт, которые уже показывались в этом окне выбора (стартовый offer + предыдущие реролы). На вес редкости (RarityWeight тира) НЕ влияет — редкость карты выбирается отдельно. 1.0 — фича выключена; 0.05 — карты-повторы почти не выпадают; 0.0 — полностью исключаются (если все карты тира уже видели, тир целиком выпадает из кандидатов). Список 'видел' очищается, когда игрок забирает награду." ) )
	float RerollSeenWeightMultiplier = 0.05f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Debug" )
	bool bDebugShowAllCards = false;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Status Effects",
		meta = ( ClampMin = "0.0", ClampMax = "99.0",
			ToolTip = "Глобальный кап на суммарное замедление движения (UStatusEffect_Slow) на враге, когда стакаются разные источники. В процентах. 90 = враг не может стать медленнее, чем 10% от своей оригинальной скорости." ) )
	float GlobalSlowCapPercent = 90.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Status Effects",
		meta = ( ClampMin = "0.0", ClampMax = "99.0",
			ToolTip = "Глобальный кап на суммарное замедление атаки (UStatusEffect_AttackSlow) на враге, когда стакаются разные источники. В процентах. 90 = AttackCooldown максимум +90% (≈1.9× от оригинала)." ) )
	float GlobalAttackSlowCapPercent = 90.f;

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

	UFUNCTION( BlueprintPure, Category = "Card Pool|Wave Overrides" )
	float GetRarityWeightMultiplierForWave( int32 waveNumber, ECardRarity rarity ) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid( class FDataValidationContext& context ) const override;
#endif
};
