#pragma once

#include "Cards/CardEffect.h"

#include "CoreMinimal.h"

#include "CardEffect_HealNeighborWalls.generated.h"

UCLASS( BlueprintType, meta = ( DisplayName = "Effect: Heal Neighbor Walls" ) )
class LORDS_FRONTIERS_API UCardEffect_HealNeighborWalls : public UCardEffect
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect", meta = ( ClampMin = "0.0" ) )
	float Radius = 150.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect", meta = ( ClampMin = "1" ) )
	int32 HealAmount = 2;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	bool bHealClosestOnly = true;

	virtual void Execute_Implementation( const FCardEffectContext& context ) override;
	virtual bool RequiresRuntimeRegistration_Implementation() const override
	{
		return true;
	}
	virtual FText GetDisplayText_Implementation() const override;
};
