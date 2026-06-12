#pragma once

#include "Cards/CardTypes.h"

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "CardSelectionWidget.generated.h"

class UCardWidget;
class UCardDataAsset;
class UCardSubsystem;
class UButton;
class UTextBlock;
class UHorizontalBox;
class UScrollBox;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnCardSelectionComplete, const TArray<UCardDataAsset*>&, SelectedCards );

/**
 * UCardSelectionWidget
 *
 * Full-screen overlay for card selection after wave.
 */
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UCardSelectionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, Category = "Card Selection" )
	void ShowWithChoices( const FCardChoice& choice );

	UFUNCTION( BlueprintCallable, Category = "Card Selection" )
	void ShowWithCards( const TArray<UCardDataAsset*>& cards, int32 numToSelect );

	UFUNCTION( BlueprintCallable, Category = "Card Selection" )
	void Hide();

	UFUNCTION( BlueprintPure, Category = "Card Selection" )
	TArray<UCardDataAsset*> GetSelectedCards() const;

	UFUNCTION( BlueprintPure, Category = "Card Selection" )
	int32 GetSelectedCount() const
	{
		return SelectedCardWidgets_.Num();
	}

	UFUNCTION( BlueprintPure, Category = "Card Selection" )
	int32 GetRequiredSelectionCount() const
	{
		return CardsToSelect_;
	}

	UFUNCTION( BlueprintPure, Category = "Card Selection" )
	bool CanConfirm() const
	{
		return SelectedCardWidgets_.Num() >= CardsToSelect_;
	}

	UPROPERTY( BlueprintAssignable, Category = "Card Selection|Events" )
	FOnCardSelectionComplete OnSelectionComplete;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION( BlueprintImplementableEvent, Category = "Card Selection" )
	void OnSelectionCountChanged( int32 currentCount, int32 requiredCount );

	UFUNCTION( BlueprintImplementableEvent, Category = "Card Selection" )
	void OnWidgetShown();

	UFUNCTION( BlueprintImplementableEvent, Category = "Card Selection" )
	void OnWidgetHiding();

	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UScrollBox> CardsScrollBox;

	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UHorizontalBox> CardsContainer;

	UPROPERTY( BlueprintReadOnly, meta = ( BindWidget ) )
	TObjectPtr<UButton> ConfirmButton;

	/** Optional label inside ConfirmButton; C++ fills it from ST_GameStrings (CardSelection.Confirm). */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> ConfirmButtonText;

	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> RerollButton;

	/** Optional label inside RerollButton («Обновить»); C++ fills it from ST_GameStrings (CardSelection.Reroll). */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> RerollButtonText;

	/** Optional reroll cost value next to the label — just the number, the resource icon lives in the WBP. */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> RerollCostText;

	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> SelectionCountText;

	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> WaveText;

	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UImage> BackgroundOverlay;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Card Selection|Config" )
	TSubclassOf<UCardWidget> CardWidgetClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Card Selection|Config",
		meta = ( ToolTip = "Optional per-rarity widget overrides. If a rarity is not present, CardWidgetClass is used as a fallback." ) )
	TMap<ECardRarity, TSubclassOf<UCardWidget>> RarityCardWidgetClasses;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Card Selection|Config" )
	FText SelectionCountFormat = FText::FromString( TEXT( "{0} / {1}" ) );

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Card Selection|Config" )
	bool bAutoApplyCards = true;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Card Selection|Config" )
	float CardSpacing = 20.0f;


	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Card Selection|Config",
		meta = ( ToolTip = "Z-order used when adding the widget to the viewport. Keep below HUD elements (e.g. ResourcesPanel) that should remain visible." ) )
	int32 ViewportZOrder = 10;

	UFUNCTION( BlueprintImplementableEvent, Category = "Card Selection" )
	void OnRerollAvailabilityChanged( bool bCanReroll, int32 cost );

	UFUNCTION( BlueprintImplementableEvent, Category = "Card Selection" )
	void OnCardsRerolled();

private:
	UPROPERTY()
	TArray<TObjectPtr<UCardWidget>> CardWidgets_;

	UPROPERTY()
	TArray<TObjectPtr<UCardWidget>> SelectedCardWidgets_;

	int32 CardsToSelect_ = 2;
	int32 CurrentWaveNumber_ = 0;
	int32 RerollCount_ = 0;

	TWeakObjectPtr<UCardSubsystem> CachedCardSubsystem_;

	void CreateCardWidgets( const TArray<UCardDataAsset*>& cards );
	void ClearCardWidgets();

	UFUNCTION()
	void HandleCardClicked( UCardWidget* cardWidget );

	UFUNCTION()
	void HandleConfirmClicked();

	UFUNCTION()
	void HandleRerollClicked();

	void UpdateSelectionUI();
	void UpdateTitleText();
	void UpdateRerollUI();

	UCardSubsystem* GetCardSubsystem();
};