#pragma once

#include "Cards/CardEffect.h"
#include "Components/Attack/AttackRangedComponent.h"

#include "CoreMinimal.h"

#include "CardEffect_SetTargetPriority.generated.h"

UCLASS( BlueprintType, meta = ( DisplayName = "Effect: Set Target Priority" ) )
class LORDS_FRONTIERS_API UCardEffect_SetTargetPriority : public UCardEffect
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	ETowerTargetPriority Priority = ETowerTargetPriority::LowestHP;

	virtual void Apply_Implementation( const FCardEffectContext& context ) override;
	virtual void Revert_Implementation( const FCardEffectContext& context ) override;
	virtual FText GetDisplayText_Implementation() const override;
};
