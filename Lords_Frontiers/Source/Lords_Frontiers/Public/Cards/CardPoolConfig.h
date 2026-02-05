#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "CardPoolConfig.generated.h"

class UCardDataAsset;

/**
 UCardPoolConfig
 
 Configuration asset for the card system.
 
 Usage:
 1. Create in Editor: Right-click -> Miscellaneous -> Data Asset -> CardPoolConfig
 2. Add cards to CardPool array
 3. Optionally configure StartingCards
 4. Assign to CardSubsystem via GameMode or directly
 */
UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UCardPoolConfig : public UDataAsset
{
	GENERATED_BODY()

public:

	/**
	 All cards available for random selection during gameplay.
	 Cards with higher SelectionWeight appear more frequently.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Card Pool" )
	TArray<TObjectPtr<UCardDataAsset>> CardPool;

	/**
	 Cards applied silently at game start.
	 Use for:
	 Difficulty modifiers
	 Starting bonuses
	 Game mode specific effects
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Starting Cards" )
	TArray<TObjectPtr<UCardDataAsset>> StartingCards;

	/**
	 * How many cards to show for selection after each wave.
	 * Typical: 3-5
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Selection", meta = ( ClampMin = "1", ClampMax = "10" ) )
	int32 CardsToOffer = 4;

	/**
	 How many cards the player must pick from the offered set.
	 Typical: 1-2
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Selection", meta = ( ClampMin = "1", ClampMax = "5" ) )
	int32 CardsToSelect = 2;

	/** Returns total number of cards in pool */
	UFUNCTION( BlueprintPure, Category = "Card Pool" )
	int32 GetPoolSize() const
	{
		return CardPool.Num();
	}

	/** Returns true if pool has enough cards for selection */
	UFUNCTION( BlueprintPure, Category = "Card Pool" )
	bool HasEnoughCards() const
	{
		return CardPool.Num() >= CardsToOffer;
	}

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid( class FDataValidationContext& context ) const override;
#endif
};
