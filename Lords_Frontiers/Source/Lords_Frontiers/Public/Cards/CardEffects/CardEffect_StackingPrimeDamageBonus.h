#pragma once

#include "Cards/CardEffect.h"

#include "CoreMinimal.h"

#include "CardEffect_StackingPrimeDamageBonus.generated.h"

UCLASS( BlueprintType, meta = ( DisplayName = "Effect: Stacking Prime Damage Bonus" ) )
class LORDS_FRONTIERS_API UCardEffect_StackingPrimeDamageBonus : public UCardEffect
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect",
		meta = ( ToolTip = "How much % damage each stack adds. Step=2 + 5 stacks = +10% on next shot." ) )
	float StepPerStackPercent = 2.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect",
		meta = ( ClampMin = "1",
			ToolTip = "Maximum number of stacks. Step * MaxStacks = total cap %." ) )
	int32 MaxStacks = 6;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	bool bResetOnTargetChange = true;

	virtual void Execute_Implementation( const FCardEffectContext& context ) override;
	virtual void Revert_Implementation( const FCardEffectContext& context ) override;
	virtual bool RequiresRuntimeRegistration_Implementation() const override
	{
		return true;
	}
	virtual FText GetDisplayText_Implementation() const override;
};
