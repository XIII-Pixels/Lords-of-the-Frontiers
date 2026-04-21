#pragma once

#include "Cards/CardEffect.h"

#include "CoreMinimal.h"

#include "CardEffect_GlobalEconomy.generated.h"

UENUM( BlueprintType )
enum class ECardEconomyOp : uint8
{
	ProductionBonus			UMETA( DisplayName = "Production Bonus" ),
	MaintenanceReduction	UMETA( DisplayName = "Maintenance Reduction" ),
};

UCLASS( BlueprintType, meta = ( DisplayName = "Effect: Global Economy" ) )
class LORDS_FRONTIERS_API UCardEffect_GlobalEconomy : public UCardEffect
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	ECardEconomyOp Operation = ECardEconomyOp::ProductionBonus;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	EResourceTargetType ResourceTarget = EResourceTargetType::All;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	int32 Delta = 0;

	virtual void Apply_Implementation( const FCardEffectContext& context ) override;
	virtual void Revert_Implementation( const FCardEffectContext& context ) override;
	virtual FText GetDisplayText_Implementation() const override;
	virtual bool IsGlobalEffect_Implementation() const override
	{
		return true;
	}

private:
	void ApplyDelta( const FCardEffectContext& context, int32 signedDelta ) const;
};
