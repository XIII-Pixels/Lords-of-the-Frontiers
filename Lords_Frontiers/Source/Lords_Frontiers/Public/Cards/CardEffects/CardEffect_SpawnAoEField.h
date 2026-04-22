#pragma once

#include "Cards/CardEffect.h"
#include "Cards/CardEffects/CardEffect_AoEExplosion.h"

#include "CoreMinimal.h"

#include "CardEffect_SpawnAoEField.generated.h"

class ACardAoEField;
class UStatusEffectDef;

UCLASS( BlueprintType, meta = ( DisplayName = "Effect: Spawn AoE Field" ) )
class LORDS_FRONTIERS_API UCardEffect_SpawnAoEField : public UCardEffect
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	TSubclassOf<ACardAoEField> FieldClass;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect", meta = ( ClampMin = "0.0" ) )
	float Radius = 150.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect", meta = ( ClampMin = "0.1" ) )
	float Duration = 5.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect", meta = ( ClampMin = "0.1" ) )
	float TickInterval = 1.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	int32 DamagePerTick = 0;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	TObjectPtr<UStatusEffectDef> StatusPerTick;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	EAoECenterOrigin CenterOrigin = EAoECenterOrigin::EventInstigator;

	virtual void Execute_Implementation( const FCardEffectContext& context ) override;
	virtual bool RequiresRuntimeRegistration_Implementation() const override
	{
		return true;
	}
	virtual FText GetDisplayText_Implementation() const override;
};
