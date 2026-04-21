#pragma once

#include "Cards/CardEffect.h"

#include "CoreMinimal.h"

#include "CardEffect_AoEExplosion.generated.h"

class UStatusEffectDef;

UENUM( BlueprintType )
enum class EAoECenterOrigin : uint8
{
	EventInstigator	UMETA( DisplayName = "Event Instigator (target/victim)" ),
	OwnerBuilding	UMETA( DisplayName = "Owner Building (self)" ),
};

UCLASS( BlueprintType, meta = ( DisplayName = "Effect: AoE Explosion" ) )
class LORDS_FRONTIERS_API UCardEffect_AoEExplosion : public UCardEffect
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect", meta = ( ClampMin = "0.0" ) )
	float Radius = 200.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	int32 Damage = 0;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect",
		meta = ( ToolTip = "If > 0, damage is owner AttackDamage * this multiplier. Ignored when 0." ) )
	float DamageMultiplierOfOwner = 0.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	TObjectPtr<UStatusEffectDef> StatusToApply;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	EAoECenterOrigin CenterOrigin = EAoECenterOrigin::EventInstigator;

	virtual void Execute_Implementation( const FCardEffectContext& context ) override;
	virtual bool RequiresRuntimeRegistration_Implementation() const override
	{
		return true;
	}
	virtual FText GetDisplayText_Implementation() const override;
};
