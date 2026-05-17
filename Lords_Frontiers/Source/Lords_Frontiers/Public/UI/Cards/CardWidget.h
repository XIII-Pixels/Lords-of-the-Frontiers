#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "CardWidget.generated.h"

class UCardDataAsset;
class UImage;
class URichTextBlock;
class UButton;
class USizeBox;

/**
 * Delegate fired when card is clicked.
 * Used by CardSelectionWidget to track selections.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnCardClicked, UCardWidget*, CardWidget );

/**
 * UCardWidget
 *
 * Minimal card visual:
 * - BuildingIconImage  (Постройка) — UCardDataAsset::BuildingIcon
 * - FeatureIconImage   (Особенность) — UCardDataAsset::FeatureIcon
 * - CardNameText       (URichTextBlock) — UCardDataAsset::CardName
 * - DescriptionText    (URichTextBlock) — UCardDataAsset::BuildDescription()
 * Plus a click button. Selection visuals are handled in Blueprint via
 * OnSelectionChanged.
 */
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Initializes the widget with card data.
	 *
	 * @param cardData - Card to display. Must be valid.
	 */
	UFUNCTION( BlueprintCallable, Category = "Card" )
	void SetCardData( UCardDataAsset* cardData );

	/**
	 * Returns the card data this widget displays.
	 */
	UFUNCTION( BlueprintPure, Category = "Card" )
	UCardDataAsset* GetCardData() const
	{
		return CardData_;
	}

	/**
	 * Sets the selection state of the card.
	 * Drives the SelectedScale render transform and broadcasts
	 * OnSelectionChanged so Blueprint can handle visuals.
	 *
	 * @param bSelected - True to show selected state
	 */
	UFUNCTION( BlueprintCallable, Category = "Card" )
	void SetSelected( bool bSelected );

	/**
	 * Returns current selection state.
	 */
	UFUNCTION( BlueprintPure, Category = "Card" )
	bool IsSelected() const
	{
		return bIsSelected_;
	}

	/**
	 * Enables or disables interaction with the card.
	 * Disabled cards cannot be clicked/selected.
	 */
	UFUNCTION( BlueprintCallable, Category = "Card" )
	void SetInteractionEnabled( bool bEnabled );

	/**
	 * Fired when card is clicked.
	 * Parent widget should subscribe to handle selection logic.
	 */
	UPROPERTY( BlueprintAssignable, Category = "Card|Events" )
	FOnCardClicked OnCardClicked;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/**
	 * Called when card data is set.
	 * Override in Blueprint to update custom UI elements.
	 */
	UFUNCTION( BlueprintImplementableEvent, Category = "Card" )
	void OnCardDataSet();

	/**
	 * Called when selection state changes.
	 * Override in Blueprint for custom selection visuals.
	 *
	 * @param bSelected - New selection state
	 */
	UFUNCTION( BlueprintImplementableEvent, Category = "Card" )
	void OnSelectionChanged( bool bSelected );

	/**
	 * Called when mouse enters the card.
	 * Override for hover effects.
	 */
	UFUNCTION( BlueprintImplementableEvent, Category = "Card" )
	void OnCardHovered();

	/**
	 * Called when mouse leaves the card.
	 */
	UFUNCTION( BlueprintImplementableEvent, Category = "Card" )
	void OnCardUnhovered();

	/** Main clickable button covering the entire card */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidget ) )
	TObjectPtr<UButton> CardButton;

	/** Building icon («Постройка»), driven by UCardDataAsset::BuildingIcon. */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UImage> BuildingIconImage;

	/** Feature icon («Особенность»), driven by UCardDataAsset::FeatureIcon. */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UImage> FeatureIconImage;

	/** Effect frame badge: shows Fire/Slow/Fire+Slow icon based on UCardDataAsset::EffectFrames. */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UImage> EffectFrameIconImage;

	/** Texture shown when the card has only the Fire effect frame. */
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Card|Appearance|EffectFrame" )
	TObjectPtr<UTexture2D> EffectFrameFireTexture = nullptr;

	/** Texture shown when the card has only the Slow effect frame. */
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Card|Appearance|EffectFrame" )
	TObjectPtr<UTexture2D> EffectFrameSlowTexture = nullptr;

	/** Texture shown when the card has both the Fire and Slow effect frames. */
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Card|Appearance|EffectFrame" )
	TObjectPtr<UTexture2D> EffectFrameSlowFireTexture = nullptr;

	/** Card name (rich text — supports inline decorators/styles) */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidget ) )
	TObjectPtr<URichTextBlock> CardNameText;

	/** Card description (rich text — supports inline decorators/styles) */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidget ) )
	TObjectPtr<URichTextBlock> DescriptionText;

	/** Render scale applied while the card is selected. Uses render transform so siblings don't shift. */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Card|Appearance",
		meta = ( ClampMin = "1.0", UIMin = "1.0" ) )
	float SelectedScale = 1.15f;

private:
	/** Current card data */
	UPROPERTY()
	TObjectPtr<UCardDataAsset> CardData_;

	/** Current selection state */
	bool bIsSelected_ = false;

	/** Current interaction enabled state */
	bool bIsInteractionEnabled_ = true;

	/** Button click handler */
	UFUNCTION()
	void HandleButtonClicked();

	/** Button hover handlers */
	UFUNCTION()
	void HandleButtonHovered();

	UFUNCTION()
	void HandleButtonUnhovered();

	/** Updates all visuals from card data */
	void UpdateCardVisuals();
};
