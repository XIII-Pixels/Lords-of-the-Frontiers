#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "CardPoolConfig.generated.h"

class UCardDataAsset;

UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UCardPoolConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Card Pool" )
	TArray<TObjectPtr<UCardDataAsset>> CardPool;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Starting Cards" )
	TArray<TObjectPtr<UCardDataAsset>> StartingCards;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Selection", meta = ( ClampMin = "1", ClampMax = "10" ) )
	int32 CardsToOffer = 4;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Selection", meta = ( ClampMin = "1", ClampMax = "5" ) )
	int32 CardsToSelect = 2;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Debug" )
	bool bDebugShowAllCards = false;

	UFUNCTION( BlueprintPure, Category = "Card Pool" )
	int32 GetPoolSize() const
	{
		return CardPool.Num();
	}

	UFUNCTION( BlueprintPure, Category = "Card Pool" )
	bool HasEnoughCards() const
	{
		return CardPool.Num() >= CardsToOffer;
	}

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid( class FDataValidationContext& context ) const override;
#endif
};
