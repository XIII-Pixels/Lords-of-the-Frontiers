#pragma once

#include "Cards/CardTypes.h"
#include "Cards/Visuals/CardVisualTypes.h"

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "CardEffect.generated.h"

UCLASS( Abstract, Blueprintable, BlueprintType,
	meta = ( DisplayName = "Card Effect" ) )
class LORDS_FRONTIERS_API UCardEffect : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Visuals" )
	FCardVisualConfig VisualConfig;

	UFUNCTION( BlueprintNativeEvent, Category = "Card|Effect" )
	void Apply( const FCardEffectContext& context );
	virtual void Apply_Implementation( const FCardEffectContext& context )
	{
	}

	UFUNCTION( BlueprintNativeEvent, Category = "Card|Effect" )
	void Revert( const FCardEffectContext& context );
	virtual void Revert_Implementation( const FCardEffectContext& context )
	{
	}

	UFUNCTION( BlueprintNativeEvent, Category = "Card|Effect" )
	void Execute( const FCardEffectContext& context );
	virtual void Execute_Implementation( const FCardEffectContext& context )
	{
	}

	UFUNCTION( BlueprintNativeEvent, BlueprintPure, Category = "Card|Effect" )
	FText GetDisplayText() const;
	virtual FText GetDisplayText_Implementation() const
	{
		return FText::GetEmpty();
	}

	UFUNCTION( BlueprintNativeEvent, BlueprintPure, Category = "Card|Effect" )
	bool RequiresRuntimeRegistration() const;
	virtual bool RequiresRuntimeRegistration_Implementation() const
	{
		return false;
	}

	UFUNCTION( BlueprintNativeEvent, BlueprintPure, Category = "Card|Effect" )
	bool IsGlobalEffect() const;
	virtual bool IsGlobalEffect_Implementation() const
	{
		return false;
	}

	UFUNCTION( BlueprintNativeEvent, BlueprintPure, Category = "Card|Effect" )
	bool WantsTargetChangedReset() const;
	virtual bool WantsTargetChangedReset_Implementation() const
	{
		return false;
	}

	/**
	 * If true, the effect itself is responsible for playing VisualConfig (likely per
	 * target / per spawned actor / etc.). The dispatcher in UCardSubsystem and
	 * UCardEffectHostComponent will skip the default PlayOneShot for this effect.
	 *
	 * Use this for effects like AoE explosions that need to play the configured
	 * visual on every affected enemy, not just on the kill victim.
	 */
	UFUNCTION( BlueprintNativeEvent, BlueprintPure, Category = "Card|Effect" )
	bool HandlesOwnVisuals() const;
	virtual bool HandlesOwnVisuals_Implementation() const
	{
		return false;
	}
};
