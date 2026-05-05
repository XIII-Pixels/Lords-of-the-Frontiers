#pragma once

#include "Cards/CardTypes.h"

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "CardCollectionWidget.generated.h"

class UButton;
class UCardDataAsset;
class UCardSubsystem;
class UCardWidget;
class UImage;
class UPanelWidget;
class UWidget;
class UWidgetAnimation;

/**
 * UCardCollectionWidget
 *
 * "Book" widget that displays every card the player has acquired so far.
 *
 * Layout:
 *   - Cards are split across two pages (left + right) like the screenshot.
 *   - Each page is a grid of CardsPerRow * RowsPerPage cells.
 *   - Both grid dimensions are exposed in DefaultsOnly so designers can tune
 *     the layout without touching code or BP graphs.
 *
 * Ordering rules (driven by UCardSubsystem::GetAcquisitionLog):
 *   - Acquisition order is preserved for *first* occurrences.
 *   - Duplicates of the same card are grouped immediately next to the first
 *     occurrence of that card, regardless of when they were taken later.
 *
 * Empty slots:
 *   - Cells that are not occupied by an acquired card show QuestionSlotClass.
 *     A class must be configured — there is no built-in fallback.
 *
 * Cards in the book reuse the same UCardWidget hierarchy as the reward
 * selection screen so that visuals stay consistent across the two flows.
 *
 * Sizing:
 *   - bUseManualCardSize = false: every cell is wrapped in a SizeBox sized
 *     from the page grid's rendered geometry. Width  = pageW / CardsPerRow,
 *     Height = pageH / RowsPerPage, multiplied by AutoCardFillFactor and
 *     constrained to AutoCardAspectRatio so cards never get stretched.
 *     Values > 1 oversize the SizeBox past its grid cell — Center alignment
 *     in the slot lets it overflow into neighbours, producing real overlap
 *     while keeping text/icons sharp (the widget re-rasterizes at the new
 *     size, unlike a render-transform scale).
 *   - bUseManualCardSize = true:  every cell is wrapped in a SizeBox sized
 *     to ManualCardSize.
 *
 * Animations:
 *   - BookOpenAnim     plays when the book is shown.
 *   - BookCloseAnim    plays when the book is hidden (waited on before remove).
 *   - OpenButtonAnim   plays on the open button (intro / pulse / etc).
 *   These are bound by name from the widget BP via meta = ( BindWidgetAnim ).
 */
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UCardCollectionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Pulls the current acquisition log from CardSubsystem and rebuilds both pages. */
	UFUNCTION( BlueprintCallable, Category = "Card Collection" )
	void RefreshFromSubsystem();

	/** Rebuilds the grid from an explicit list (already in desired display order). */
	UFUNCTION( BlueprintCallable, Category = "Card Collection" )
	void SetCards( const TArray<UCardDataAsset*>& cards );

	/** Shows the book and plays the open animation. */
	UFUNCTION( BlueprintCallable, Category = "Card Collection" )
	void OpenBook();

	/** Plays the close animation, then hides the book. */
	UFUNCTION( BlueprintCallable, Category = "Card Collection" )
	void CloseBook();

	/** Returns true while the book is on screen. */
	UFUNCTION( BlueprintPure, Category = "Card Collection" )
	bool IsOpen() const
	{
		return bIsOpen_;
	}

	/** Number of card cells per row, per page. */
	UFUNCTION( BlueprintPure, Category = "Card Collection|Layout" )
	int32 GetCardsPerRow() const
	{
		return FMath::Max( 1, CardsPerRow );
	}

	/** Number of rows per page. */
	UFUNCTION( BlueprintPure, Category = "Card Collection|Layout" )
	int32 GetRowsPerPage() const
	{
		return FMath::Max( 1, RowsPerPage );
	}

	/** Total cells across both pages. */
	UFUNCTION( BlueprintPure, Category = "Card Collection|Layout" )
	int32 GetTotalCells() const
	{
		return GetCardsPerRow() * GetRowsPerPage() * 2;
	}

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick( const FGeometry& myGeometry, float deltaTime ) override;

	/** BP hook fired after cells have been (re)built. */
	UFUNCTION( BlueprintImplementableEvent, Category = "Card Collection" )
	void OnCollectionRebuilt( int32 acquiredCount, int32 totalCells );

	/** BP hook fired when the book finishes opening. */
	UFUNCTION( BlueprintImplementableEvent, Category = "Card Collection" )
	void OnBookOpened();

	/** BP hook fired when the book finishes closing. */
	UFUNCTION( BlueprintImplementableEvent, Category = "Card Collection" )
	void OnBookClosed();

	/** Left page grid (UUniformGridPanel or any UPanelWidget). */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidget ) )
	TObjectPtr<UPanelWidget> LeftPageGrid;

	/** Right page grid (UUniformGridPanel or any UPanelWidget). */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidget ) )
	TObjectPtr<UPanelWidget> RightPageGrid;

	/** Optional button rendered next to the HUD that opens the book. */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> OpenBookButton;

	/** Optional close button inside the book. */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> CloseBookButton;

	/**
	 * Full-screen invisible button placed behind the book.
	 * Clicking outside the book lands on this button and closes the book.
	 */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> BackdropButton;

	/** Root container that holds the book (hidden until OpenBook is called). */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UWidget> BookRoot;

	/** Book cover/background image — hidden until OpenBook is called. */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UImage> BookImage;

	/** Open-pages image (the paper sheets) — hidden until OpenBook is called. */
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UImage> PagesImage;

	/** Plays when the book opens. Bind in the widget BP. */
	UPROPERTY( Transient, meta = ( BindWidgetAnimOptional ) )
	TObjectPtr<UWidgetAnimation> BookOpenAnim;

	/** Plays when the book closes. Bind in the widget BP. */
	UPROPERTY( Transient, meta = ( BindWidgetAnimOptional ) )
	TObjectPtr<UWidgetAnimation> BookCloseAnim;

	/** Plays on the open-book button (e.g. intro / idle pulse). Bind in the widget BP. */
	UPROPERTY( Transient, meta = ( BindWidgetAnimOptional ) )
	TObjectPtr<UWidgetAnimation> OpenButtonAnim;

	/** Card visual class used for occupied cells. Same hierarchy as the reward screen. */
	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Card Collection|Cells" )
	TSubclassOf<UCardWidget> CardWidgetClass;

	/** Optional per-rarity overrides (mirrors UCardSelectionWidget so visuals stay in sync). */
	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Card Collection|Cells" )
	TMap<ECardRarity, TSubclassOf<UCardWidget>> RarityCardWidgetClasses;

	/** Widget shown for empty cells (the "?" placeholder). Must be set — there is no fallback. */
	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Card Collection|Cells" )
	TSubclassOf<UUserWidget> QuestionSlotClass;

	/**
	 * If false, cells (cards and question placeholders) stretch to fill their grid slot.
	 * If true, every cell is wrapped in a SizeBox forced to ManualCardSize.
	 */
	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Card Collection|Sizing" )
	bool bUseManualCardSize = false;

	/** Manual cell size in slate units. Applied to both cards and question placeholders. */
	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Card Collection|Sizing",
		meta = ( EditCondition = "bUseManualCardSize", ClampMin = "1.0", UIMin = "1.0" ) )
	FVector2D ManualCardSize = FVector2D( 160.0f, 220.0f );

	/**
	 * Card aspect ratio (width / height) used for auto sizing.
	 * The widget picks the largest size that fits a grid cell while
	 * preserving this ratio, so cards never look stretched.
	 */
	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Card Collection|Sizing",
		meta = ( EditCondition = "!bUseManualCardSize", ClampMin = "0.1", UIMin = "0.1" ) )
	float AutoCardAspectRatio = 160.0f / 220.0f;

	/**
	 * Fraction of the cell budget the cards/questions actually occupy in auto mode.
	 * 1.0 packs cells edge-to-edge; lower values leave breathing room around them;
	 * values above 1.0 oversize cells past their grid budget (they will overlap).
	 */
	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Card Collection|Sizing",
		meta = ( EditCondition = "!bUseManualCardSize", ClampMin = "0.05", ClampMax = "5.0",
			UIMin = "0.05", UIMax = "5.0" ) )
	float AutoCardFillFactor = 0.7f;

	/**
	 * If true, every cell is also wrapped in a ScaleBox so its inner content
	 * scales uniformly to fill the SizeBox. Lets card/question BPs use any
	 * internal layout (Canvas, fixed-size Image, etc.) without breaking sizing.
	 */
	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Card Collection|Sizing" )
	bool bWrapCellsInScaleBox = true;

	/** Cells per row per page. */
	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Card Collection|Layout",
		meta = ( ClampMin = "1", UIMin = "1" ) )
	int32 CardsPerRow = 3;

	/** Rows per page. */
	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Card Collection|Layout",
		meta = ( ClampMin = "1", UIMin = "1" ) )
	int32 RowsPerPage = 3;

	/** If true, RefreshFromSubsystem is called automatically when the book opens. */
	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Card Collection|Behavior" )
	bool bAutoRefreshOnOpen = true;

	/** If true, refreshes the book whenever new cards are applied through CardSubsystem. */
	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Card Collection|Behavior" )
	bool bAutoRefreshOnCardsApplied = true;

	/** If true, plays OpenButtonAnim once when the widget is constructed. */
	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "Card Collection|Behavior" )
	bool bPlayButtonAnimOnConstruct = true;

private:
	UPROPERTY()
	TArray<TObjectPtr<class USizeBox>> CellSizeBoxes_;

	UPROPERTY()
	TArray<TObjectPtr<UWidget>> SpawnedCells_;

	bool bIsOpen_ = false;
	FVector2D LastAppliedCellSize_ = FVector2D::ZeroVector;

	TWeakObjectPtr<UCardSubsystem> CachedCardSubsystem_;

	UCardSubsystem* GetCardSubsystem();

	/** Produces the final display order: first-occurrence order, with duplicates grouped. */
	static TArray<UCardDataAsset*> BuildDisplayOrder( const TArray<UCardDataAsset*>& acquisitionLog );

	void RebuildCells( const TArray<UCardDataAsset*>& displayOrder );
	void ClearCells();

	UWidget* CreateCardCellWidget( UCardDataAsset* cardData );
	UWidget* CreateEmptyCellWidget();

	/** Wraps the cell in a SizeBox so we can drive its size from manual config or auto-fit. */
	class USizeBox* WrapCellForSizing( UWidget* cell );

	void AddCellToPage( UPanelWidget* page, UWidget* cell, int32 cellIndexOnPage );
	void SetBookVisualsVisible( bool bVisible );

	/** Reads grid geometry and pushes the resulting cell size into every SizeBox. */
	void UpdateAutoCellSize();

	/** Computes the largest (w, h) that fits in the page while honoring CardsPerRow x RowsPerPage. */
	FVector2D ComputeAutoCellSize( const FVector2D& pageSize ) const;

	UFUNCTION()
	void HandleOpenButtonClicked();

	UFUNCTION()
	void HandleCloseButtonClicked();

	UFUNCTION()
	void HandleBackdropClicked();

	UFUNCTION()
	void HandleCloseAnimFinished();

	UFUNCTION()
	void HandleCardsApplied( const TArray<UCardDataAsset*>& appliedCards );

	FWidgetAnimationDynamicEvent CloseAnimDelegate_;
};
