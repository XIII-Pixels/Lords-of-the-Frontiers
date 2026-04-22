#pragma once

#include "Cards/CardEffect.h"

#include "CoreMinimal.h"

#include "CardEffect_OneShotStatModifier.generated.h"

UCLASS( BlueprintType, meta = ( DisplayName = "Effect: One-Shot Stat Modifier" ) )
class LORDS_FRONTIERS_API UCardEffect_OneShotStatModifier : public UCardEffect
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect",
		meta = ( GetOptions = "GetModifiableStatNames" ) )
	FName StatName = NAME_None;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	float Delta = 0.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect",
		meta = ( ClampMin = "1" ) )
	int32 ApplyEveryN = 1;

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
