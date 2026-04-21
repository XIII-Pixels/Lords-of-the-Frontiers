#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "StatusEffectDef.generated.h"

struct FActiveStatus;

UCLASS( Abstract, Blueprintable, BlueprintType,
	meta = ( DisplayName = "Status Effect" ) )
class LORDS_FRONTIERS_API UStatusEffectDef : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Status" )
	FName StatusTag = NAME_None;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Status",
		meta = ( ClampMin = "0.1" ) )
	float Duration = 3.f;

	virtual void OnApply( AActor* owner, FActiveStatus& state ) const
	{
	}

	virtual void OnTick( AActor* owner, FActiveStatus& state ) const
	{
	}

	virtual void OnRemove( AActor* owner, FActiveStatus& state ) const
	{
	}

	virtual float GetTickInterval() const
	{
		return 0.f;
	}
};

UCLASS( BlueprintType, meta = ( DisplayName = "Status: Burn" ) )
class LORDS_FRONTIERS_API UStatusEffect_Burn : public UStatusEffectDef
{
	GENERATED_BODY()

public:
	UStatusEffect_Burn();

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Status",
		meta = ( ClampMin = "1" ) )
	int32 DamagePerTick = 1;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Status",
		meta = ( ClampMin = "0.1" ) )
	float TickInterval = 1.f;

	virtual void OnTick( AActor* owner, FActiveStatus& state ) const override;
	virtual float GetTickInterval() const override
	{
		return TickInterval;
	}
};

UCLASS( BlueprintType, meta = ( DisplayName = "Status: Slow" ) )
class LORDS_FRONTIERS_API UStatusEffect_Slow : public UStatusEffectDef
{
	GENERATED_BODY()

public:
	UStatusEffect_Slow();

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Status",
		meta = ( ClampMin = "0", ClampMax = "99" ) )
	float SpeedReductionPercent = 25.f;

	virtual void OnApply( AActor* owner, FActiveStatus& state ) const override;
	virtual void OnRemove( AActor* owner, FActiveStatus& state ) const override;
};
