#pragma once

#include "Cards/CardEffect.h"

#include "CoreMinimal.h"

#include "CardEffect_StackingBuff.generated.h"

UCLASS( BlueprintType, meta = ( DisplayName = "Effect: Stacking Buff" ) )
class LORDS_FRONTIERS_API UCardEffect_StackingBuff : public UCardEffect
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect",
		meta = ( GetOptions = "GetModifiableStatNames" ) )
	FName StatName = NAME_None;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	float StepPerTrigger = 1.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect",
		meta = ( ClampMin = "0.0" ) )
	float MaxAccumulated = 0.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	bool bResetOnTargetChange = true;

	virtual void Execute_Implementation( const FCardEffectContext& context ) override;
	virtual void Revert_Implementation( const FCardEffectContext& context ) override;
	virtual bool RequiresRuntimeRegistration_Implementation() const override
	{
		return true;
	}
	virtual bool WantsTargetChangedReset_Implementation() const override
	{
		return bResetOnTargetChange;
	}
	virtual FText GetDisplayText_Implementation() const override;

	UFUNCTION()
	static TArray<FString> GetModifiableStatNames();
};
