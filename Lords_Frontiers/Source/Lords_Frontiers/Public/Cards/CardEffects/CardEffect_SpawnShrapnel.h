#pragma once

#include "Cards/CardEffect.h"

#include "CoreMinimal.h"

#include "CardEffect_SpawnShrapnel.generated.h"

class ABaseProjectile;

UCLASS( BlueprintType, meta = ( DisplayName = "Effect: Spawn Shrapnel" ) )
class LORDS_FRONTIERS_API UCardEffect_SpawnShrapnel : public UCardEffect
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect", meta = ( ClampMin = "1" ) )
	int32 ShrapnelCount = 4;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect",
		meta = ( ClampMin = "0.0",
			ToolTip = "Damage as fraction of owner's AttackDamage. 0.35 = 35% per shard." ) )
	float DamageMultiplier = 0.35f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect", meta = ( ClampMin = "1.0" ) )
	float Range = 600.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect", meta = ( ClampMin = "1.0" ) )
	float Speed = 1200.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect", meta = ( ClampMin = "0.0" ) )
	float ShardSplashRadius = 0.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect",
		meta = ( ClampMin = "0.0", ClampMax = "360.0",
			ToolTip = "Total spread arc in degrees. 360 = full ring, 90 = forward fan." ) )
	float SpreadDegrees = 360.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect",
		meta = ( ToolTip = "Optional override. If null, uses tower's AttackRangedComponent::ProjectileClass_." ) )
	TSubclassOf<ABaseProjectile> ShardProjectileClass;

	virtual void Execute_Implementation( const FCardEffectContext& context ) override;
	virtual bool RequiresRuntimeRegistration_Implementation() const override
	{
		return true;
	}
	virtual FText GetDisplayText_Implementation() const override;
};
