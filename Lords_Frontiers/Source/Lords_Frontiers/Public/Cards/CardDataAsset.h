#pragma once

#include "Cards/CardTypes.h"

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "CardDataAsset.generated.h"

class ABuilding;

UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UCardDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Identity" )
	FName CardID = NAME_None;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Display",
		meta = ( ToolTip = "Id of this card's texts in the ST_Cards string table: the card shows Card.Name.<Id> and Card.Description.<Id> when those keys exist. None = use this asset's name as the Id. CardName/Description below are only the fallback for missing keys." ) )
	FName LocalizationId = NAME_None;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Display",
		meta = ( ToolTip = "Fallback name, shown only when ST_Cards has no Card.Name.<Id> entry. Localized cards keep their texts in Content/Localization/ST_Cards.csv." ) )
	FText CardName;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Display",
		meta = ( MultiLine = true,
			ToolTip = "Rendered as Rich Text in the card widget. Use <decorator>...</decorator> tags as configured on the WBP." ) )
	FText Description;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Display",
		meta = ( ToolTip = "«Постройка» — building/silhouette icon shown on the card widget." ) )
	TObjectPtr<UTexture2D> BuildingIcon = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Display",
		meta = ( ToolTip = "«Особенность» — feature/specialty icon shown on the card widget." ) )
	TObjectPtr<UTexture2D> FeatureIcon = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Display",
		meta = ( ToolTip = "Effect frame badges to show on the card widget. Empty = no frame; Fire = Effect_Frame_Fire; Slow = Effect_Frame_Slow; Fire+Slow = Effect_Frame_Slow_Fire." ) )
	TArray<ECardEffectFrame> EffectFrames;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Display" )
	ECardRarity Rarity = ECardRarity::Common;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Display" )
	ECardCategory Category = ECardCategory::None;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Display",
		meta = ( ToolTip = "Free-form tags. Other cards can boost or dampen the weight of every card sharing one of these tags via TagWeightMultipliers_Up/Down." ) )
	TArray<FName> Tags;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Events" )
	TArray<FCardEvent> Events;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Selection",
		meta = ( ClampMin = "0.01" ) )
	float BaseWeight = 100.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Selection" )
	bool bCanStack = true;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Selection" )
	bool bUnlockedByDefault = true;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Selection|Unlock" )
	TArray<TObjectPtr<UCardDataAsset>> UnlockedBy;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Selection|Influence" )
	TMap<TObjectPtr<UCardDataAsset>, float> WeightMultipliers_Up;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Selection|Influence" )
	TMap<TObjectPtr<UCardDataAsset>, float> WeightMultipliers_Down;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Selection|Influence" )
	TMap<ECardCategory, float> CategoryWeightMultipliers_Up;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Selection|Influence" )
	TMap<ECardCategory, float> CategoryWeightMultipliers_Down;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Selection|Influence",
		meta = ( ToolTip = "While this card is in history, every card whose Tags array contains the listed tag has its pick weight multiplied by the value (>1 = boost). Stacks per applied copy up to MaxStacksForWeightInfluence. Multiple entries compose multiplicatively." ) )
	TMap<FName, float> TagWeightMultipliers_Up;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Selection|Influence" )
	TMap<FName, float> TagWeightMultipliers_Down;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Selection|Influence" )
	TSet<TObjectPtr<UCardDataAsset>> ExcludedCards;

	UFUNCTION( BlueprintPure, Category = "Card" )
	FLinearColor GetRarityColor() const;

	/** Card name for display: ST_Cards entry Card.Name.<Id> when present, otherwise CardName. */
	UFUNCTION( BlueprintPure, Category = "Card" )
	FText GetCardName() const;

	UFUNCTION( BlueprintPure, Category = "Card" )
	FText BuildDescription() const;

	UFUNCTION( BlueprintPure, Category = "Card" )
	FText GetTargetDescription() const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid( class FDataValidationContext& context ) const override;
#endif

private:
	/** Looks up <keyPrefix><LocalizationId or asset name> in ST_Cards; falls back to inlineText. */
	FText ResolveLocalizedText( const TCHAR* keyPrefix, const FText& inlineText ) const;
};
