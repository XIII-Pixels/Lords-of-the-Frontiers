#pragma once

#include "Cards/CardEffect.h"

#include "CoreMinimal.h"

#include "CardEffect_ConditionalStatBonus.generated.h"

class UCardCondition;

UCLASS( BlueprintType, meta = ( DisplayName = "Effect: Conditional Stat Bonus" ) )
class LORDS_FRONTIERS_API UCardEffect_ConditionalStatBonus : public UCardEffect
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect",
		meta = ( GetOptions = "GetModifiableStatNames" ) )
	FName StatName = NAME_None;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	float BonusAmount = 0.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect",
		meta = ( AllowAbstract = "false" ) )
	TObjectPtr<UCardCondition> ActiveWhile;

	virtual void Execute_Implementation( const FCardEffectContext& context ) override;
	virtual void Revert_Implementation( const FCardEffectContext& context ) override;
	virtual bool RequiresRuntimeRegistration_Implementation() const override
	{
		return true;
	}
	virtual FText GetDisplayText_Implementation() const override;

	UFUNCTION()
	static TArray<FString> GetModifiableStatNames();
};
