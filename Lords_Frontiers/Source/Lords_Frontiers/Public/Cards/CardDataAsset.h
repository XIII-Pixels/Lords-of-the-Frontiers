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

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Display" )
	FText CardName;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Display",
		meta = ( MultiLine = true ) )
	FText Description;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Display" )
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Display" )
	ECardRarity Rarity = ECardRarity::Common;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Display" )
	ECardCategory Category = ECardCategory::None;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Feedback" )
	bool bShowIconOnTrigger = true;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Feedback" )
	TObjectPtr<UTexture2D> FeedbackIconOverride = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Events" )
	TArray<FCardEvent> Events;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Selection",
		meta = ( ClampMin = "0.01" ) )
	float BaseWeight = 100.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Selection" )
	bool bCanStack = true;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Selection" )
	bool bUnlockedByDefault = true;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Selection|Influence" )
	TMap<TObjectPtr<UCardDataAsset>, float> WeightMultipliers_Up;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Selection|Influence" )
	TMap<TObjectPtr<UCardDataAsset>, float> WeightMultipliers_Down;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Card|Selection|Influence" )
	TSet<TObjectPtr<UCardDataAsset>> ExcludedCards;

	UFUNCTION( BlueprintPure, Category = "Card" )
	FLinearColor GetRarityColor() const;

	UFUNCTION( BlueprintPure, Category = "Card" )
	FText BuildDescription() const;

	UFUNCTION( BlueprintPure, Category = "Card" )
	FText GetTargetDescription() const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid( class FDataValidationContext& context ) const override;
#endif
};
