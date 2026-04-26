#pragma once

#include "Cards/CardEffect.h"

#include "CoreMinimal.h"

#include "CardEffect_ModifyMaintenance.generated.h"

UCLASS( BlueprintType, meta = ( DisplayName = "Effect: Modify Maintenance" ) )
class LORDS_FRONTIERS_API UCardEffect_ModifyMaintenance : public UCardEffect
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	EResourceTargetType ResourceTarget = EResourceTargetType::All;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	int32 Delta = 0;

	virtual void Apply_Implementation( const FCardEffectContext& context ) override;
	virtual void Revert_Implementation( const FCardEffectContext& context ) override;
	virtual FText GetDisplayText_Implementation() const override;

private:
	void ApplyDelta( const FCardEffectContext& context, int32 signedDelta ) const;
};
