#pragma once

#include "Cards/CardEffect.h"

#include "CoreMinimal.h"

#include "CardEffect_StatModifier.generated.h"

UCLASS( BlueprintType, meta = ( DisplayName = "Effect: Stat Modifier" ) )
class LORDS_FRONTIERS_API UCardEffect_StatModifier : public UCardEffect
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect",
		meta = ( GetOptions = "GetModifiableStatNames" ) )
	FName StatName = NAME_None;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Effect" )
	float Delta = 0.f;

	virtual void Apply_Implementation( const FCardEffectContext& context ) override;
	virtual void Revert_Implementation( const FCardEffectContext& context ) override;
	virtual FText GetDisplayText_Implementation() const override;

	UFUNCTION()
	static TArray<FString> GetModifiableStatNames();

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid( class FDataValidationContext& context ) const override;
#endif

private:
	void ApplyDelta( const FCardEffectContext& context, float signedDelta ) const;
};
