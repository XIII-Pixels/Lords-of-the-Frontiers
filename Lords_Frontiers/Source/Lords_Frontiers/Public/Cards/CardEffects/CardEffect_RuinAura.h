#pragma once

#include "Cards/CardEffect.h"

#include "CoreMinimal.h"

#include "CardEffect_RuinAura.generated.h"

class UStatusEffectDef;

UCLASS( BlueprintType, meta = ( DisplayName = "Effect: Ruin Aura" ) )
class LORDS_FRONTIERS_API UCardEffect_RuinAura : public UCardEffect
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect", meta = ( ClampMin = "0.0" ) )
	float Radius = 150.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect", meta = ( ClampMin = "0" ) )
	int32 DamagePerTick = 0;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	TObjectPtr<UStatusEffectDef> StatusToApply;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect|Debug" )
	bool bDebugDrawRadius = false;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect|Debug",
		meta = ( EditCondition = "bDebugDrawRadius" ) )
	FColor DebugColor = FColor::Orange;

	virtual void Execute_Implementation( const FCardEffectContext& context ) override;
	virtual bool RequiresRuntimeRegistration_Implementation() const override
	{
		return true;
	}
	virtual FText GetDisplayText_Implementation() const override;
};
