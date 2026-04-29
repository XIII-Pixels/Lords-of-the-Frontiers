#pragma once

#include "Cards/CardEffect.h"

#include "CoreMinimal.h"

#include "CardEffect_PrimeNextShotDamageBonus.generated.h"

UCLASS( BlueprintType, meta = ( DisplayName = "Effect: Prime Next Shot Damage Bonus" ) )
class LORDS_FRONTIERS_API UCardEffect_PrimeNextShotDamageBonus : public UCardEffect
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	float DamageBonusPercent = 35.f;

	virtual void Execute_Implementation( const FCardEffectContext& context ) override;
	virtual bool RequiresRuntimeRegistration_Implementation() const override
	{
		return true;
	}
	virtual FText GetDisplayText_Implementation() const override;
};
