#pragma once

#include "Cards/CardEffect.h"

#include "CoreMinimal.h"

#include "CardEffect_AuraStacker.generated.h"

class UStatusEffectDef;

UCLASS( BlueprintType, meta = ( DisplayName = "Effect: Aura Stacker" ) )
class LORDS_FRONTIERS_API UCardEffect_AuraStacker : public UCardEffect
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect",
		meta = ( GetOptions = "GetModifiableStatNames" ) )
	FName StatName = NAME_None;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	float StepPerEnemy = 0.05f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect", meta = ( ClampMin = "0.0" ) )
	float MaxAccumulated = 0.5f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect",
		meta = ( ToolTip = "null = count any enemy in range" ) )
	TObjectPtr<UStatusEffectDef> RequiredStatus;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect",
		meta = ( ClampMin = "0.0",
			ToolTip = "0 = use owner AttackRange" ) )
	float Radius = 0.f;

	virtual void Execute_Implementation( const FCardEffectContext& context ) override;
	virtual void Revert_Implementation( const FCardEffectContext& context ) override;
	virtual bool RequiresRuntimeRegistration_Implementation() const override
	{
		return true;
	}
	virtual FText GetDisplayText_Implementation() const override;

	UFUNCTION()
	static TArray<FString> GetModifiableStatNames();
};
