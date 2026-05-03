#pragma once

#include "Cards/CardTypes.h"

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "CardRarityPoolConfig.generated.h"

class UCardDataAsset;

/**
 * UCardRarityPoolConfig
 *
 * One DataAsset per rarity tier. UCardPoolConfig::RarityPools references these.
 * The pool resolver first picks a tier (weighted by RarityWeight), then picks
 * a card inside the tier (weighted by UCardDataAsset::BaseWeight + synergies).
 */
UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UCardRarityPoolConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Rarity" )
	ECardRarity Rarity = ECardRarity::Common;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Rarity",
		meta = ( ClampMin = "0.0",
			ToolTip = "Weight of this tier when the resolver picks which rarity to roll from for an offer slot." ) )
	float RarityWeight = 100.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Cards" )
	TArray<TObjectPtr<UCardDataAsset>> Cards;

	UFUNCTION( BlueprintPure, Category = "Rarity" )
	int32 GetCardCount() const
	{
		return Cards.Num();
	}

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid( class FDataValidationContext& context ) const override;
#endif
};
