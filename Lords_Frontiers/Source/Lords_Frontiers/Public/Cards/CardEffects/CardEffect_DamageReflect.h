#pragma once

#include "Cards/CardEffect.h"

#include "CoreMinimal.h"

#include "CardEffect_DamageReflect.generated.h"

UCLASS( BlueprintType, meta = ( DisplayName = "Effect: Damage Reflect" ) )
class LORDS_FRONTIERS_API UCardEffect_DamageReflect : public UCardEffect
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect",
		meta = ( ClampMin = "0.0", ClampMax = "100.0" ) )
	float ReflectPercent = 25.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect",
		meta = ( ClampMin = "1",
			ToolTip = "Reflect fires on every N-th damage event. 1 = every hit." ) )
	int32 EveryNthHit = 1;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect",
		meta = ( ClampMin = "1" ) )
	int32 MinReflectDamage = 1;

	virtual void Execute_Implementation( const FCardEffectContext& context ) override;
	virtual bool RequiresRuntimeRegistration_Implementation() const override
	{
		return true;
	}
	virtual FText GetDisplayText_Implementation() const override;
};
