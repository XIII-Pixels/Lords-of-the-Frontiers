#pragma once

#include "Cards/CardEffect.h"

#include "CoreMinimal.h"

#include "CardEffect_SpawnExtraProjectile.generated.h"

UCLASS( BlueprintType, meta = ( DisplayName = "Effect: Spawn Extra Projectile" ) )
class LORDS_FRONTIERS_API UCardEffect_SpawnExtraProjectile : public UCardEffect
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect", meta = ( ClampMin = "1" ) )
	int32 ExtraProjectileCount = 1;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect", meta = ( ClampMin = "0.0" ) )
	float DamageMultiplier = 1.f;

	virtual void Execute_Implementation( const FCardEffectContext& context ) override;
	virtual bool RequiresRuntimeRegistration_Implementation() const override
	{
		return true;
	}
	virtual FText GetDisplayText_Implementation() const override;
};
