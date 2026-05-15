#pragma once

#include "Cards/CardTypes.h"
#include "Cards/Visuals/CardVisualTypes.h"
#include "Lords_Frontiers/Public/EntityStats.h"

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "CardEffect.generated.h"

class UCardVFXAsset;

UCLASS( Abstract, Blueprintable, BlueprintType,
	meta = ( DisplayName = "Card Effect" ) )
class LORDS_FRONTIERS_API UCardEffect : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Visuals",
		meta = ( DisplayName = "VFX (DA)",
			ToolTip = "DA с пресетом визуала (один DA = один VFX). Если задан — используется он. Если пуст — fallback на встроенный VisualConfig ниже." ) )
	TObjectPtr<UCardVFXAsset> VFX;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Visuals",
		meta = ( DisplayName = "Встроенный VisualConfig (legacy)",
			ToolTip = "Используется только если поле VFX выше не задано. Для новых эффектов предпочтительно VFX-DA." ) )
	FCardVisualConfig VisualConfig;

	const FCardVisualConfig& GetVisualConfig() const;

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

	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "Card|Effect|Preview" )
	void PreviewBuildingTooltip(
	    const ABuilding* building, FEntityStats& InOutStats, FResourceProduction& InOutBuildingCost,
	    FResourceProduction& InOutMaintenanceCost
	) const;

	virtual void PreviewBuildingTooltip_Implementation(												// MUST override to get actual data to UI Buttons
	    const ABuilding* building, FEntityStats& InOutStats, FResourceProduction& InOutBuildingCost,
	    FResourceProduction& InOutMaintenanceCost
	) const
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

	UFUNCTION( BlueprintNativeEvent, BlueprintPure, Category = "Card|Effect" )
	bool HandlesOwnVisuals() const;
	virtual bool HandlesOwnVisuals_Implementation() const
	{
		return false;
	}
};
