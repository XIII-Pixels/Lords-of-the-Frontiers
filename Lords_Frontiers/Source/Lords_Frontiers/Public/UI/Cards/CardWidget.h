#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "CardWidget.generated.h"

class UCardDataAsset;
class UImage;
class UTextBlock;
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
 * Widget displaying a single card with:
 * - Card icon
 * - Card name
 * - Description
 * - Selection state (normal/selected border)
 *
 * Usage:
 * 1. Create Blueprint child: WBP_CardWidget
 * 2. Add UI elements and bind to exposed properties
 * 3. Call SetCardData() to populate
 * 4. Subscribe to OnCardClicked for selection handling
 *
 * The widget handles click detection and visual state changes.
 * Parent widget (CardSelectionWidget) manages selection logic.
 */
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Initializes the widget with card data.
	 * Call this after creating the widget.
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
	 * Changes border texture between normal and selected.
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

	/** Card border image - switches between normal/selected textures */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidget ) )
	TObjectPtr<UImage> BorderImage;

	/** Card background image */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UImage> BackgroundImage;

	/** Card icon/artwork */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UImage> IconImage;

	/** Card name text */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> CardNameText;

	/** Card description text */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> DescriptionText;

	/** Target description (which buildings affected) */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> TargetText;

	/** Border texture for normal (unselected) state */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Card|Appearance" )
	TObjectPtr<UTexture2D> NormalBorderTexture;

	/** Border texture for selected state */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Card|Appearance" )
	TObjectPtr<UTexture2D> SelectedBorderTexture;

	/** Background texture for the card */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Card|Appearance" )
	TObjectPtr<UTexture2D> CardBackgroundTexture;

	/** Color tint for normal state */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Card|Appearance" )
	FLinearColor NormalTint = FLinearColor::White;

	/** Color tint for hovered state */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Card|Appearance" )
	FLinearColor HoveredTint = FLinearColor( 1.1f, 1.1f, 1.1f, 1.0f );

	/** Color tint for disabled state */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Card|Appearance" )
	FLinearColor DisabledTint = FLinearColor( 0.5f, 0.5f, 0.5f, 1.0f );

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

	/** Updates border image based on current state */
	void UpdateBorderVisual();

	/** Updates all visuals from card data */
	void UpdateCardVisuals();
};
