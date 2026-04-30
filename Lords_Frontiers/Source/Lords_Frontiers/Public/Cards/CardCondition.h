#pragma once

#include "Cards/CardTypes.h"

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "CardCondition.generated.h"

UCLASS( Abstract, Blueprintable, BlueprintType,
	meta = ( DisplayName = "Card Condition" ) )
class LORDS_FRONTIERS_API UCardCondition : public UDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintNativeEvent, BlueprintPure, Category = "Card|Condition" )
	bool IsMet( const FCardEffectContext& context ) const;
	virtual bool IsMet_Implementation( const FCardEffectContext& context ) const
	{
		return true;
	}

	UFUNCTION( BlueprintNativeEvent, BlueprintPure, Category = "Card|Condition" )
	FText GetDisplayText() const;
	virtual FText GetDisplayText_Implementation() const
	{
		return FText::GetEmpty();
	}
};
