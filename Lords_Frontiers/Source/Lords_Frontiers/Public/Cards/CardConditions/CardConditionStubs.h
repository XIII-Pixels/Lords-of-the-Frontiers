#pragma once

#include "Cards/CardCondition.h"

#include "CoreMinimal.h"

#include "CardConditionStubs.generated.h"

UCLASS( BlueprintType, meta = ( DisplayName = "Condition: After N Shots" ) )
class LORDS_FRONTIERS_API UCardCondition_AfterNShots : public UCardCondition
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Condition",
		meta = ( ClampMin = "1" ) )
	int32 N = 5;

	virtual bool IsMet_Implementation( const FCardEffectContext& context ) const override
	{
		return false;
	}

	virtual FText GetDisplayText_Implementation() const override
	{
		return FText::FromString( FString::Printf( TEXT( "Every %d shots" ), N ) );
	}
};

UCLASS( BlueprintType, meta = ( DisplayName = "Condition: After N Hits" ) )
class LORDS_FRONTIERS_API UCardCondition_AfterNHits : public UCardCondition
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Condition",
		meta = ( ClampMin = "1" ) )
	int32 N = 5;

	virtual bool IsMet_Implementation( const FCardEffectContext& context ) const override
	{
		return false;
	}

	virtual FText GetDisplayText_Implementation() const override
	{
		return FText::FromString( FString::Printf( TEXT( "Every %d hits" ), N ) );
	}
};

UCLASS( BlueprintType, meta = ( DisplayName = "Condition: Shot Dealt Damage" ) )
class LORDS_FRONTIERS_API UCardCondition_ShotDealtDamage : public UCardCondition
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Condition" )
	bool bDealtDamage = true;

	virtual bool IsMet_Implementation( const FCardEffectContext& context ) const override
	{
		return false;
	}

	virtual FText GetDisplayText_Implementation() const override
	{
		return bDealtDamage
			? FText::FromString( TEXT( "On damaging shot" ) )
			: FText::FromString( TEXT( "On non-damaging shot" ) );
	}
};

UCLASS( BlueprintType, meta = ( DisplayName = "Condition: Target Has Status" ) )
class LORDS_FRONTIERS_API UCardCondition_TargetHasStatus : public UCardCondition
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Condition" )
	TSoftObjectPtr<UObject> RequiredStatus;

	virtual bool IsMet_Implementation( const FCardEffectContext& context ) const override
	{
		return false;
	}

	virtual FText GetDisplayText_Implementation() const override
	{
		return FText::FromString( TEXT( "Target has status" ) );
	}
};

UCLASS( BlueprintType, meta = ( DisplayName = "Condition: Enemy In Radius" ) )
class LORDS_FRONTIERS_API UCardCondition_EnemyInRadius : public UCardCondition
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Condition",
		meta = ( ClampMin = "0.0" ) )
	float Radius = 300.f;

	virtual bool IsMet_Implementation( const FCardEffectContext& context ) const override
	{
		return false;
	}

	virtual FText GetDisplayText_Implementation() const override
	{
		return FText::FromString( FString::Printf( TEXT( "Enemy within %.0f" ), Radius ) );
	}
};
