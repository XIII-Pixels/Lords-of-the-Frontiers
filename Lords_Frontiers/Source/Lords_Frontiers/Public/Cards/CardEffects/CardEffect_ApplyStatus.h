#pragma once

#include "Cards/CardEffect.h"

#include "CoreMinimal.h"

#include "CardEffect_ApplyStatus.generated.h"

class UStatusEffectDef;

UCLASS( BlueprintType, meta = ( DisplayName = "Effect: Apply Status" ) )
class LORDS_FRONTIERS_API UCardEffect_ApplyStatus : public UCardEffect
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	TObjectPtr<UStatusEffectDef> StatusToApply;

	virtual void Execute_Implementation( const FCardEffectContext& context ) override;
	virtual bool RequiresRuntimeRegistration_Implementation() const override
	{
		return true;
	}
	virtual FText GetDisplayText_Implementation() const override;
};
