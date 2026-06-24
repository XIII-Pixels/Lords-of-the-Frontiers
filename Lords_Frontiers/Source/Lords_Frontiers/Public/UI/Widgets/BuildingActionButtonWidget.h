#pragma once

#include "Sound/AudioTags.h"

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "CoreMinimal.h"
#include "sound/AudioEvent.h"
#include "sound/AudioEventSource.h"

#include "BuildingActionButtonWidget.generated.h"

class ABuilding;

UENUM( BlueprintType )
enum class EBuildingActionType : uint8
{
	Relocate,
	Remove
};

UCLASS( Blueprintable )
class LORDS_FRONTIERS_API UBuildingActionButtonWidget : public UUserWidget, public IAudioEventSource
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings" )
	EBuildingActionType Action = EBuildingActionType::Relocate;

	UPROPERTY( BlueprintReadOnly, meta = ( BindWidget ) )
	TObjectPtr<UButton> Button;

	UPROPERTY( BlueprintReadOnly, meta = ( BindWidget ) )
	TObjectPtr<UTextBlock> CostText;

	/** Optional action label; C++ fills it from ST_GameStrings (Building.Action.Relocate/Remove). */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> LabelText;

	UPROPERTY( EditAnywhere, Category = "Settings|UI" )
	float DisabledOpacity = 0.4f;

	void RefreshFor( ABuilding* building );

	virtual FOnAudioEvent& GetOnAudioEvent() override
	{
		return OnAudioEvent_;
	}

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	bool CanActOn( const ABuilding* building ) const;
	void ApplyCostText( const ABuilding* building );
	void PerformAction( ABuilding* building );
	bool IsCombatPhase() const;

	FGameplayTag GetClickSoundTag() const;
	FGameplayTag GetHoverSoundTag() const;

	UFUNCTION() void HandleClicked();
	UFUNCTION() void HandleHovered();

	TWeakObjectPtr<ABuilding> Building_;

	FOnAudioEvent OnAudioEvent_;
};
