#include "UI/Cards/CardCollectionWidget.h"

#include "Cards/CardDataAsset.h"
#include "Cards/CardSubsystem.h"
#include "Cards/CardTypes.h"
#include "Framework/Application/SlateApplication.h"
#include "UI/Cards/CardWidget.h"
#include "UI/CursorAnim/CursorAnimFrameImage.h"

#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/ScaleBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/AudioTags.h"
#include "sound/SoundEffectManager.h"

void UCardCollectionWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	// Runs in the UMG designer as well as at runtime, so the configured toggle-button brush
	// and count colour are visible while editing instead of the default white button.
	UpdateToggleButtonVisual();
	UpdateAcquiredCountText();
}

void UCardCollectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( OpenBookButton )
	{
		OpenBookButton->OnClicked.AddDynamic( this, &UCardCollectionWidget::HandleOpenButtonClicked );
		OpenBookButton->OnHovered.AddDynamic( this, &UCardCollectionWidget::HandleOpenButtonHovered );
	}

	if ( CloseBookButton )
	{
		CloseBookButton->OnClicked.AddDynamic( this, &UCardCollectionWidget::HandleCloseButtonClicked );
		CloseBookButton->OnHovered.AddDynamic( this, &UCardCollectionWidget::HandleCloseButtonHovered );
	}

	if ( BackdropButton )
	{
		BackdropButton->OnClicked.AddDynamic( this, &UCardCollectionWidget::HandleBackdropClicked );
	}

	if ( NextPageButton )
	{
		NextPageButton->OnClicked.AddDynamic( this, &UCardCollectionWidget::HandleNextPageClicked );
		NextPageButton->OnHovered.AddDynamic( this, &UCardCollectionWidget::HandlePageButtonHovered );
	}

	if ( PrevPageButton )
	{
		PrevPageButton->OnClicked.AddDynamic( this, &UCardCollectionWidget::HandlePrevPageClicked );
		PrevPageButton->OnHovered.AddDynamic( this, &UCardCollectionWidget::HandlePageButtonHovered );
	}

	// Open/close animations follow the same forward/reverse contract as the building and
	// construction panels: the same finished callback is bound to both animations.
	VisibilityAnimDelegate_.BindDynamic( this, &UCardCollectionWidget::OnVisibilityAnimFinished );
	if ( ShowAnim )
	{
		BindToAnimationFinished( ShowAnim, VisibilityAnimDelegate_ );
	}
	if ( HideAnim )
	{
		BindToAnimationFinished( HideAnim, VisibilityAnimDelegate_ );
	}

	SetBookVisualsVisible( false );
	bIsOpen_ = false;
	bWantsVisible_ = false;
	UpdateToggleButtonVisual();
	UpdateAcquiredCountText();
	UpdatePageNavVisual();

	if ( bPlayButtonAnimOnConstruct && OpenButtonAnim )
	{
		PlayAnimation( OpenButtonAnim );
	}

	if ( bAutoRefreshOnCardsApplied )
	{
		if ( UCardSubsystem* subsystem = GetCardSubsystem() )
		{
			subsystem->OnCardsApplied.AddDynamic( this, &UCardCollectionWidget::HandleCardsApplied );
		}
	}

	// Sound
	if ( const UWorld* world = GetWorld() )
	{
		if ( const UGameInstance* gameInstance = UGameplayStatics::GetGameInstance( world ) )
		{
			if ( USoundEffectManager* sfxManager = gameInstance->GetSubsystem<USoundEffectManager>() )
			{
				sfxManager->RegisterObject( this );
			}
		}
	}
}

void UCardCollectionWidget::NativeDestruct()
{
	if ( IsValid( OpenBookButton ) )
	{
		OpenBookButton->OnClicked.RemoveDynamic( this, &UCardCollectionWidget::HandleOpenButtonClicked );
		OpenBookButton->OnHovered.RemoveDynamic( this, &UCardCollectionWidget::HandleOpenButtonHovered );
	}

	if ( IsValid( CloseBookButton ) )
	{
		CloseBookButton->OnClicked.RemoveDynamic( this, &UCardCollectionWidget::HandleCloseButtonClicked );
		CloseBookButton->OnHovered.RemoveDynamic( this, &UCardCollectionWidget::HandleCloseButtonHovered );
	}

	if ( IsValid( BackdropButton ) )
	{
		BackdropButton->OnClicked.RemoveDynamic( this, &UCardCollectionWidget::HandleBackdropClicked );
	}

	if ( IsValid( NextPageButton ) )
	{
		NextPageButton->OnClicked.RemoveDynamic( this, &UCardCollectionWidget::HandleNextPageClicked );
		NextPageButton->OnHovered.RemoveDynamic( this, &UCardCollectionWidget::HandlePageButtonHovered );
	}

	if ( IsValid( PrevPageButton ) )
	{
		PrevPageButton->OnClicked.RemoveDynamic( this, &UCardCollectionWidget::HandlePrevPageClicked );
		PrevPageButton->OnHovered.RemoveDynamic( this, &UCardCollectionWidget::HandlePageButtonHovered );
	}

	if ( UCardSubsystem* subsystem = CachedCardSubsystem_.Get() )
	{
		subsystem->OnCardsApplied.RemoveDynamic( this, &UCardCollectionWidget::HandleCardsApplied );
	}

	if ( ShowAnim )
	{
		UnbindFromAnimationFinished( ShowAnim, VisibilityAnimDelegate_ );
	}
	if ( HideAnim )
	{
		UnbindFromAnimationFinished( HideAnim, VisibilityAnimDelegate_ );
	}

	ClearCells();

	// Sound
	if ( const UWorld* world = GetWorld() )
	{
		if ( const UGameInstance* gameInstance = UGameplayStatics::GetGameInstance( world ) )
		{
			if ( USoundEffectManager* sfxManager = gameInstance->GetSubsystem<USoundEffectManager>() )
			{
				sfxManager->UnregisterObject( this );
			}
		}
	}

	Super::NativeDestruct();
}

void UCardCollectionWidget::RefreshFromSubsystem()
{
	UCardSubsystem* subsystem = GetCardSubsystem();
	if ( !subsystem )
	{
		ApplyDisplayOrder( {} );
		return;
	}

	const TArray<UCardDataAsset*>& log = subsystem->GetAcquisitionLog();
	ApplyDisplayOrder( BuildDisplayOrder( log ) );
}

void UCardCollectionWidget::SetCards( const TArray<UCardDataAsset*>& cards )
{
	ApplyDisplayOrder( BuildDisplayOrder( cards ) );
}

void UCardCollectionWidget::OpenBook()
{
	if ( bIsOpen_ )
	{
		return;
	}

	bIsOpen_ = true;
	bWantsVisible_ = true;

	if ( bAutoRefreshOnOpen )
	{
		RefreshFromSubsystem();
	}

	SetBookVisualsVisible( true );
	PlayVisibilityAnim( true );
	UpdateToggleButtonVisual();
	UpdateAcquiredCountText();
	UpdatePageNavVisual();

	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_TOGGLECARDBOOK_CLICKED } );

	OnBookOpened();
}

void UCardCollectionWidget::CloseBook()
{
	if ( !bIsOpen_ )
	{
		return;
	}

	bIsOpen_ = false;
	bWantsVisible_ = false;
	ClearZoomedCard();
	UpdateToggleButtonVisual();
	UpdateAcquiredCountText();
	UpdatePageNavVisual();

	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_TOGGLECARDBOOK_CLICKED } );

	PlayVisibilityAnim( false );

	// No animation to wait on — collapse immediately and fire the closed hook now.
	if ( !HideAnim && !ShowAnim )
	{
		SetBookVisualsVisible( false );
		OnBookClosed();
	}
}

TArray<UCardDataAsset*> UCardCollectionWidget::BuildDisplayOrder( const TArray<UCardDataAsset*>& acquisitionLog )
{
	// Group duplicates: keep the index of the first appearance as the anchor for that card,
	// and append every subsequent copy directly after the existing run.
	TArray<UCardDataAsset*> result;
	result.Reserve( acquisitionLog.Num() );

	TMap<UCardDataAsset*, int32> lastIndexByCard;

	for ( UCardDataAsset* card : acquisitionLog )
	{
		if ( !card )
		{
			continue;
		}

		if ( int32* lastIndex = lastIndexByCard.Find( card ) )
		{
			const int32 insertAt = *lastIndex + 1;
			result.Insert( card, insertAt );

			// Shift the cached last-index of every card whose run starts at or after the insertion point.
			for ( TPair<UCardDataAsset*, int32>& pair : lastIndexByCard )
			{
				if ( pair.Value >= insertAt )
				{
					++pair.Value;
				}
			}

			lastIndexByCard[card] = insertAt;
		}
		else
		{
			lastIndexByCard.Add( card, result.Num() );
			result.Add( card );
		}
	}

	return result;
}

void UCardCollectionWidget::ApplyDisplayOrder( const TArray<UCardDataAsset*>& displayOrder )
{
	FullDisplayOrder_.Reset( displayOrder.Num() );
	for ( UCardDataAsset* card : displayOrder )
	{
		FullDisplayOrder_.Add( card );
	}

	// Keep the player on a valid spread (e.g. after cards were removed/refreshed).
	CurrentSpread_ = FMath::Clamp( CurrentSpread_, 0, GetSpreadCount() - 1 );

	ShowCurrentSpread();
}

void UCardCollectionWidget::ShowCurrentSpread()
{
	const int32 perSpread = GetTotalCells();
	const int32 start = CurrentSpread_ * perSpread;

	TArray<UCardDataAsset*> spreadCards;
	spreadCards.Reserve( perSpread );
	for ( int32 i = 0; i < perSpread; ++i )
	{
		const int32 index = start + i;
		if ( FullDisplayOrder_.IsValidIndex( index ) )
		{
			spreadCards.Add( FullDisplayOrder_[index] );
		}
	}

	RebuildCells( spreadCards );
	UpdatePageNavVisual();
}

int32 UCardCollectionWidget::GetSpreadCount() const
{
	const int32 perSpread = GetTotalCells();
	if ( perSpread <= 0 || FullDisplayOrder_.Num() <= 0 )
	{
		return 1;
	}

	return FMath::DivideAndRoundUp( FullDisplayOrder_.Num(), perSpread );
}

void UCardCollectionWidget::UpdatePageNavVisual()
{
	const int32 spreadCount = GetSpreadCount();

	// Arrows only ever make sense while the book is open; keep them hidden when it is closed so they
	// don't linger on the HUD at startup (before the book has been opened for the first time).
	const bool bHasPrev = bIsOpen_ && CurrentSpread_ > 0;
	const bool bHasNext = bIsOpen_ && CurrentSpread_ < spreadCount - 1;

	if ( PrevPageButton )
	{
		PrevPageButton->SetVisibility( bHasPrev ? ESlateVisibility::Visible : ESlateVisibility::Collapsed );
	}
	if ( NextPageButton )
	{
		NextPageButton->SetVisibility( bHasNext ? ESlateVisibility::Visible : ESlateVisibility::Collapsed );
	}
}

void UCardCollectionWidget::RebuildCells( const TArray<UCardDataAsset*>& displayOrder )
{
	ClearCells();

	if ( !LeftPageGrid || !RightPageGrid )
	{
		UE_LOG( LogTemp, Error, TEXT( "CardCollectionWidget: LeftPageGrid / RightPageGrid not bound in widget BP" ) );
		return;
	}

	const int32 perRow = GetCardsPerRow();
	const int32 rowsPerPage = GetRowsPerPage();
	const int32 cellsPerPage = perRow * rowsPerPage;
	const int32 totalCells = cellsPerPage * 2;

	for ( int32 cellIdx = 0; cellIdx < totalCells; ++cellIdx )
	{
		UCardDataAsset* cardData = displayOrder.IsValidIndex( cellIdx ) ? displayOrder[cellIdx] : nullptr;

		UWidget* cell = cardData ? CreateCardCellWidget( cardData ) : CreateEmptyCellWidget();
		if ( !cell )
		{
			continue;
		}

		USizeBox* sizeBox = WrapCellForSizing( cell );
		if ( !sizeBox )
		{
			continue;
		}

		const bool bRightPage = cellIdx >= cellsPerPage;
		UCanvasPanel* page = bRightPage ? RightPageGrid.Get() : LeftPageGrid.Get();
		const int32 indexOnPage = bRightPage ? cellIdx - cellsPerPage : cellIdx;
		const int32 row = indexOnPage / perRow;
		const int32 column = indexOnPage % perRow;

		AddCellToPage( page, sizeBox, row, column );

		CellSizeBoxes_.Add( sizeBox );
		SpawnedCells_.Add( sizeBox );
		CellGridPositions_.Add( FIntPoint( column, row ) );

		// Track card -> wrapper so the click handler can grow the right cell.
		if ( UCardWidget* asCard = Cast<UCardWidget>( cell ) )
		{
			CardToSizeBox_.Add( asCard, sizeBox );
		}
	}

	// Fresh cells need an immediate layout pass so first paint isn't tiny.
	LastAppliedCellSize_ = FVector2D::ZeroVector;
	LastAppliedPageSize_ = FVector2D::ZeroVector;
	UpdateCellLayout();

	// Report the total acquired count across every spread, not just the cards on this spread.
	const int32 acquiredCount = FMath::Max( FullDisplayOrder_.Num(), displayOrder.Num() );
	OnCollectionRebuilt( acquiredCount, totalCells );
}

void UCardCollectionWidget::ClearCells()
{
	ClearZoomedCard();

	if ( LeftPageGrid )
	{
		LeftPageGrid->ClearChildren();
	}
	if ( RightPageGrid )
	{
		RightPageGrid->ClearChildren();
	}

	for ( TObjectPtr<UWidget>& cell : SpawnedCells_ )
	{
		if ( IsValid( cell ) )
		{
			cell->RemoveFromParent();
		}
	}
	SpawnedCells_.Empty();
	CellSizeBoxes_.Empty();
	CellGridPositions_.Empty();
	CardToSizeBox_.Empty();
	LastAppliedCellSize_ = FVector2D::ZeroVector;
	LastAppliedPageSize_ = FVector2D::ZeroVector;
}

UWidget* UCardCollectionWidget::CreateCardCellWidget( UCardDataAsset* cardData )
{
	if ( !cardData )
	{
		return nullptr;
	}

	TSubclassOf<UCardWidget> widgetClass = CardWidgetClass;
	if ( const TSubclassOf<UCardWidget>* rarityClass = RarityCardWidgetClasses.Find( cardData->Rarity );
	     rarityClass && *rarityClass )
	{
		widgetClass = *rarityClass;
	}

	if ( !widgetClass )
	{
		UE_LOG(
		    LogTemp, Warning,
		    TEXT( "CardCollectionWidget: no widget class for rarity %d and no fallback CardWidgetClass set" ),
		    static_cast<int32>( cardData->Rarity )
		);
		return nullptr;
	}

	UCardWidget* cardWidget = CreateWidget<UCardWidget>( this, widgetClass );
	if ( !cardWidget )
	{
		return nullptr;
	}

	cardWidget->SetCardData( cardData );

	if ( bAllowCardClickZoom )
	{
		cardWidget->SetInteractionEnabled( true );
		cardWidget->OnCardClicked.AddDynamic( this, &UCardCollectionWidget::HandleBookCardClicked );
	}
	else
	{
		cardWidget->SetInteractionEnabled( false );
	}

	return cardWidget;
}

UWidget* UCardCollectionWidget::CreateEmptyCellWidget()
{
	if ( !QuestionSlotClass )
	{
		UE_LOG(
		    LogTemp, Warning, TEXT( "CardCollectionWidget: QuestionSlotClass is not set — empty cells will be skipped" )
		);
		return nullptr;
	}

	return CreateWidget<UUserWidget>( this, QuestionSlotClass );
}

USizeBox* UCardCollectionWidget::WrapCellForSizing( UWidget* cell )
{
	if ( !cell || !WidgetTree )
	{
		return nullptr;
	}

	// Construct via WidgetTree so the SizeBox is registered with this widget — otherwise
	// AddChildToUniformGrid accepts it but the Slate widget never sees the override values.
	USizeBox* sizeBox = WidgetTree->ConstructWidget<USizeBox>( USizeBox::StaticClass() );
	if ( !sizeBox )
	{
		return nullptr;
	}

	const FVector2D initialSize = bUseManualCardSize ? ManualCardSize : FVector2D::ZeroVector;
	sizeBox->SetWidthOverride( initialSize.X );
	sizeBox->SetHeightOverride( initialSize.Y );

	UWidget* sizeBoxChild = cell;
	if ( bWrapCellsInScaleBox )
	{
		// ScaleToFit forces the cell's inner content to follow the SizeBox bounds even when the
		// card/question BP uses a fixed-size Canvas Panel internally.
		if ( UScaleBox* scaleBox = WidgetTree->ConstructWidget<UScaleBox>( UScaleBox::StaticClass() ) )
		{
			scaleBox->SetStretch( EStretch::ScaleToFit );
			scaleBox->AddChild( cell );
			sizeBoxChild = scaleBox;
		}
	}

	sizeBox->AddChild( sizeBoxChild );

	return sizeBox;
}

void UCardCollectionWidget::AddCellToPage( UCanvasPanel* page, UWidget* cell, int32 row, int32 column )
{
	if ( !page || !cell )
	{
		return;
	}

	UCanvasPanelSlot* slot = page->AddChildToCanvas( cell );
	if ( !slot )
	{
		return;
	}

	// Anchor at the page's top-left and use a centered alignment so the slot's Position
	// represents the *centre* of the cell. Position and Size are filled in by UpdateCellLayout
	// once the page geometry is available.
	slot->SetAnchors( FAnchors( 0.0f, 0.0f, 0.0f, 0.0f ) );
	slot->SetAlignment( FVector2D( 0.5f, 0.5f ) );
	slot->SetAutoSize( false );
	slot->SetZOrder( row * GetCardsPerRow() + column );
}

void UCardCollectionWidget::SetBookVisualsVisible( bool bVisible )
{
	const ESlateVisibility hiddenVisibility = ESlateVisibility::Collapsed;

	// BookRoot is decorative — let children handle hit-testing on their own.
	if ( BookRoot )
	{
		BookRoot->SetVisibility( bVisible ? ESlateVisibility::SelfHitTestInvisible : hiddenVisibility );
	}

	// Book art and the page canvases must catch clicks themselves so the BackdropButton
	// behind them never fires for clicks landing on the book. Bubbled-up clicks reach
	// NativeOnMouseButtonDown, where they are used to dismiss any active card zoom.
	const ESlateVisibility hitVisibility = bVisible ? ESlateVisibility::Visible : hiddenVisibility;

	if ( BookImage )
	{
		BookImage->SetVisibility( hitVisibility );
	}

	if ( PagesImage )
	{
		PagesImage->SetVisibility( hitVisibility );
	}

	if ( LeftPageGrid )
	{
		LeftPageGrid->SetVisibility( hitVisibility );
	}

	if ( RightPageGrid )
	{
		RightPageGrid->SetVisibility( hitVisibility );
	}

	// Backdrop only intercepts clicks while the book is open.
	if ( BackdropButton )
	{
		BackdropButton->SetVisibility( bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed );
	}

	// The decorative book frame/pages often live outside the optional BookRoot / BookImage /
	// PagesImage bindings (they are BindWidgetOptional and frequently left unbound), so
	// collapsing only those leaves the frame on screen after close while just the bound grids
	// hide. When the open trigger is external — this overlay has no internal OpenBookButton —
	// collapse the whole widget as a guaranteed hide; OpenBook (called from the external
	// trigger) restores it. SelfHitTestInvisible keeps the backdrop/cards clickable while open.
	if ( !OpenBookButton )
	{
		SetVisibility( bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed );
	}
}

FReply UCardCollectionWidget::NativeOnMouseButtonDown( const FGeometry& myGeometry, const FPointerEvent& mouseEvent )
{
	// Children (card buttons, the close button, the backdrop button) get the click first
	// and consume it via FReply::Handled. This override fires only when the click landed
	// on a non-button area inside the widget — book art, page canvases, empty cells, etc.
	// In that case, dismiss any active zoom but otherwise let layout continue normally.
	if ( ZoomedCard_.IsValid() )
	{
		ClearZoomedCard();
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown( myGeometry, mouseEvent );
}

void UCardCollectionWidget::NativeTick( const FGeometry& myGeometry, float deltaTime )
{
	Super::NativeTick( myGeometry, deltaTime );

	if ( !bIsOpen_ )
	{
		return;
	}

	// Layout reacts to the actual rendered size of the page Canvas Panels every frame.
	// Both manual and auto modes need this — manual mode still has to position cells based
	// on the runtime page size (cards stay evenly distributed when the page resizes).
	UpdateCellLayout();
}

void UCardCollectionWidget::UpdateCellLayout()
{
	if ( CellSizeBoxes_.Num() == 0 )
	{
		return;
	}

	// Use whichever page has a non-zero geometry (they share dimensions).
	FVector2D pageSize = FVector2D::ZeroVector;
	if ( LeftPageGrid )
	{
		pageSize = LeftPageGrid->GetCachedGeometry().GetLocalSize();
	}
	if ( pageSize.IsNearlyZero() && RightPageGrid )
	{
		pageSize = RightPageGrid->GetCachedGeometry().GetLocalSize();
	}

	if ( pageSize.IsNearlyZero() )
	{
		// Geometry not ready yet — try again next tick.
		return;
	}

	const FVector2D cardSize = bUseManualCardSize ? ManualCardSize : ComputeAutoCellSize( pageSize );

	const bool bSameCardSize = cardSize.Equals( LastAppliedCellSize_, 0.5f );
	const bool bSamePageSize = pageSize.Equals( LastAppliedPageSize_, 0.5f );
	if ( bSameCardSize && bSamePageSize )
	{
		return;
	}

	LastAppliedCellSize_ = cardSize;
	LastAppliedPageSize_ = pageSize;

	const float perRowF = static_cast<float>( GetCardsPerRow() );
	const float rowsPerPageF = static_cast<float>( GetRowsPerPage() );
	const float cellW = pageSize.X / perRowF;
	const float cellH = pageSize.Y / rowsPerPageF;

	const USizeBox* zoomedBox = ZoomedSizeBox_.Get();
	const float zoomScale = FMath::Max( 1.0f, CardClickScale );

	for ( int32 i = 0; i < CellSizeBoxes_.Num(); ++i )
	{
		USizeBox* sizeBox = CellSizeBoxes_[i];
		if ( !IsValid( sizeBox ) )
		{
			continue;
		}

		const float scale = ( sizeBox == zoomedBox ) ? zoomScale : 1.0f;
		const FVector2D appliedSize = cardSize * scale;

		sizeBox->SetWidthOverride( appliedSize.X );
		sizeBox->SetHeightOverride( appliedSize.Y );

		if ( UCanvasPanelSlot* slot = Cast<UCanvasPanelSlot>( sizeBox->Slot ) )
		{
			const FIntPoint colRow =
			    CellGridPositions_.IsValidIndex( i ) ? CellGridPositions_[i] : FIntPoint::ZeroValue;
			const FVector2D centre(
			    ( static_cast<float>( colRow.X ) + 0.5f ) * cellW, ( static_cast<float>( colRow.Y ) + 0.5f ) * cellH
			);
			slot->SetPosition( centre );
			slot->SetSize( appliedSize );
		}
	}
}

FVector2D UCardCollectionWidget::ComputeAutoCellSize( const FVector2D& pageSize ) const
{
	const float fillFactor = FMath::Clamp( AutoCardFillFactor, 0.05f, 5.0f );

	// Apply the fill factor directly to the SizeBox dimensions so the underlying widget
	// re-rasterizes at the new size (text/icons stay sharp). When the factor exceeds 1.0,
	// the SizeBox is larger than its uniform-grid cell — HAlign/VAlign Center lets it
	// overflow into neighbours, producing real overlap.
	const float cellWidthBudget = ( pageSize.X / static_cast<float>( GetCardsPerRow() ) ) * fillFactor;
	const float cellHeightBudget = ( pageSize.Y / static_cast<float>( GetRowsPerPage() ) ) * fillFactor;

	const float aspect = FMath::Max( 0.01f, AutoCardAspectRatio );

	// Fit the card aspect ratio into the cell budget — pick the dimension that constrains us.
	float width = cellWidthBudget;
	float height = width / aspect;
	if ( height > cellHeightBudget )
	{
		height = cellHeightBudget;
		width = height * aspect;
	}

	return FVector2D( width, height );
}

void UCardCollectionWidget::HandleOpenButtonClicked()
{
	// OpenBookButton is a toggle: open the book when it is closed, close it when it is open.
	if ( bIsOpen_ )
	{
		CloseBook();
	}
	else
	{
		OpenBook();
	}
}

void UCardCollectionWidget::HandleCloseButtonClicked()
{
	CloseBook();
}

void UCardCollectionWidget::HandleOpenButtonHovered()
{
	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_TOGGLECARDBOOK_HOVERED } );
}

void UCardCollectionWidget::HandleCloseButtonHovered()
{
	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_TOGGLECARDBOOK_HOVERED } );
}

void UCardCollectionWidget::HandleBackdropClicked()
{
	// The backdrop button covers the whole viewport, so clicks on the book art also reach it
	// whenever the art's hit-test is disabled (e.g. SelfHitTestInvisible in the widget BP).
	// Decide what to do based on the actual click position rather than relying on hit-test
	// alone — that way book art always keeps the book open, and outside clicks always close.
	const FVector2D screenPos = FSlateApplication::Get().GetCursorPos();

	auto isCursorOver = [&]( UWidget* widget )
	{
		if ( !widget )
		{
			return false;
		}
		const ESlateVisibility visibility = widget->GetVisibility();
		if ( visibility == ESlateVisibility::Hidden || visibility == ESlateVisibility::Collapsed )
		{
			return false;
		}
		return widget->GetCachedGeometry().IsUnderLocation( screenPos );
	};

	const bool bClickInsideBook = isCursorOver( BookImage ) || isCursorOver( PagesImage ) ||
	                              isCursorOver( LeftPageGrid ) || isCursorOver( RightPageGrid );

	if ( bClickInsideBook )
	{
		// Click landed on the book itself — keep the book open, just dismiss the zoom.
		if ( ZoomedCard_.IsValid() )
		{
			ClearZoomedCard();
		}
		return;
	}

	// True backdrop click — close the book immediately, even if a card is zoomed.
	// CloseBook resets ZoomedCard_ via ClearZoomedCard during its setup.
	CloseBook();
}

void UCardCollectionWidget::HandleNextPageClicked()
{
	if ( CurrentSpread_ >= GetSpreadCount() - 1 )
	{
		return;
	}

	++CurrentSpread_;
	ShowCurrentSpread();

	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_TOGGLECARDBOOK_CLICKED } );
}

void UCardCollectionWidget::HandlePrevPageClicked()
{
	if ( CurrentSpread_ <= 0 )
	{
		return;
	}

	--CurrentSpread_;
	ShowCurrentSpread();

	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_TOGGLECARDBOOK_CLICKED } );
}

void UCardCollectionWidget::HandlePageButtonHovered()
{
	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_TOGGLECARDBOOK_HOVERED } );
}

void UCardCollectionWidget::PlayVisibilityAnim( bool bVisible )
{
	if ( bVisible )
	{
		if ( ShowAnim )
		{
			PlayAnimationForward( ShowAnim );
		}
		else if ( HideAnim )
		{
			PlayAnimationReverse( HideAnim );
		}
	}
	else
	{
		if ( HideAnim )
		{
			PlayAnimationForward( HideAnim );
		}
		else if ( ShowAnim )
		{
			PlayAnimationReverse( ShowAnim );
		}
	}
}

void UCardCollectionWidget::OnVisibilityAnimFinished()
{
	// The close transition just finished — collapse the book art (unless a new OpenBook came in
	// mid-close and flipped bWantsVisible_ back on).
	if ( !bWantsVisible_ )
	{
		SetBookVisualsVisible( false );
		OnBookClosed();
	}
}

void UCardCollectionWidget::UpdateToggleButtonVisual()
{
	if ( !bSwapOpenButtonImage )
	{
		return;
	}

	const TCHAR* stateName = bIsOpen_ ? TEXT( "opened" ) : TEXT( "closed" );

	// Resolve the icon for this state. A directly-assigned texture wins over the brush because
	// UTexture2D references serialize reliably from the Blueprint, whereas a hand-built FSlateBrush
	// occasionally round-trips with an empty ResourceObject.
	UTexture2D* stateTexture = bIsOpen_ ? OpenButtonOpenedTexture : OpenButtonClosedTexture;
	const FSlateBrush& stateBrush = bIsOpen_ ? OpenButtonOpenedBrush : OpenButtonClosedBrush;

	// Preferred path: a dedicated image widget inside the button. We only touch this image, so the
	// button keeps its own style intact (no WidgetStyle swapping).
	if ( OpenButtonImage )
	{
		if ( stateTexture )
		{
			OpenButtonImage->SetBrushFromTexture( stateTexture );
			UE_LOG(
			    LogTemp, Log, TEXT( "CardCollectionWidget: set %s icon texture '%s' on OpenButtonImage" ), stateName,
			    *GetNameSafe( stateTexture )
			);
		}
		else if ( stateBrush.GetResourceObject() != nullptr )
		{
			OpenButtonImage->SetBrush( stateBrush );
			UE_LOG(
			    LogTemp, Log, TEXT( "CardCollectionWidget: set %s icon brush '%s' on OpenButtonImage" ), stateName,
			    *GetNameSafe( stateBrush.GetResourceObject() )
			);
		}
		else
		{
			UE_LOG( LogTemp, Log, TEXT( "CardCollectionWidget: no %s texture/brush set for OpenButtonImage" ), stateName );
		}
		return;
	}

	// Legacy fallback (no OpenButtonImage bound): swap the button's WidgetStyle instead.
	if ( !OpenBookButton )
	{
		return;
	}

	// Capture the BP-authored style once, before we ever overwrite it, so an unset icon restores the
	// designer's button look instead of stamping an empty white brush over it.
	if ( !bCapturedDefaultStyle_ )
	{
		DefaultButtonStyle_ = OpenBookButton->GetStyle();
		bCapturedDefaultStyle_ = true;
	}

	FSlateBrush iconBrush;
	bool bHasIcon = false;

	if ( stateTexture )
	{
		iconBrush.SetResourceObject( stateTexture );
		iconBrush.DrawAs = ESlateBrushDrawType::Image;
		iconBrush.ImageSize =
		    FVector2D( static_cast<float>( stateTexture->GetSizeX() ), static_cast<float>( stateTexture->GetSizeY() ) );
		bHasIcon = true;
	}
	else if ( stateBrush.GetResourceObject() != nullptr )
	{
		iconBrush = stateBrush;
		bHasIcon = true;
	}

	if ( !bHasIcon )
	{
		OpenBookButton->SetStyle( DefaultButtonStyle_ );
		return;
	}

	FButtonStyle style = DefaultButtonStyle_;
	style.SetNormal( iconBrush );
	style.SetHovered( iconBrush );
	style.SetPressed( iconBrush );
	style.SetDisabled( iconBrush );
	OpenBookButton->SetStyle( style );
}

int32 UCardCollectionWidget::GetAcquiredCardCount()
{
	if ( UCardSubsystem* subsystem = GetCardSubsystem() )
	{
		int32 count = 0;
		for ( const UCardDataAsset* card : subsystem->GetAcquisitionLog() )
		{
			if ( card )
			{
				++count;
			}
		}
		return count;
	}

	// No subsystem available — fall back to whatever was last applied to the book.
	return FullDisplayOrder_.Num();
}

void UCardCollectionWidget::UpdateAcquiredCountText()
{
	if ( !OpenButtonCountText )
	{
		return;
	}

	// In the designer there is no CardSubsystem, so keep whatever placeholder number the
	// designer typed; only drive the real count at runtime.
	if ( !IsDesignTime() )
	{
		OpenButtonCountText->SetText( FText::AsNumber( GetAcquiredCardCount() ) );
	}

	OpenButtonCountText->SetColorAndOpacity( bIsOpen_ ? OpenButtonCountColorOpened : OpenButtonCountColorClosed );
}

void UCardCollectionWidget::HandleCardsApplied( const TArray<UCardDataAsset*>& appliedCards )
{
	if ( bIsOpen_ )
	{
		RefreshFromSubsystem();
	}

	// Keep the taken-cards counter current even while the book is closed.
	UpdateAcquiredCountText();
}

void UCardCollectionWidget::HandleBookCardClicked( UCardWidget* cardWidget )
{
	if ( !cardWidget )
	{
		return;
	}

	USizeBox* targetBox = CardToSizeBox_.FindRef( cardWidget );
	if ( !targetBox )
	{
		return;
	}

	UCardWidget* currentlyZoomed = ZoomedCard_.Get();

	// Same card clicked again — un-zoom.
	if ( currentlyZoomed == cardWidget )
	{
		ApplyCellZoom( targetBox, false );
		ZoomedCard_.Reset();
		ZoomedSizeBox_.Reset();
		return;
	}

	// Different card — restore the previous one, then zoom the new one.
	if ( USizeBox* prevBox = ZoomedSizeBox_.Get() )
	{
		ApplyCellZoom( prevBox, false );
	}

	ZoomedCard_ = cardWidget;
	ZoomedSizeBox_ = targetBox;
	ApplyCellZoom( targetBox, true );
}

void UCardCollectionWidget::ClearZoomedCard()
{
	if ( USizeBox* zoomedBox = ZoomedSizeBox_.Get() )
	{
		ApplyCellZoom( zoomedBox, false );
	}
	ZoomedCard_.Reset();
	ZoomedSizeBox_.Reset();
}

void UCardCollectionWidget::ApplyCellZoom( USizeBox* sizeBox, bool bZoomed ) const
{
	if ( !sizeBox )
	{
		return;
	}

	const float scale = bZoomed ? FMath::Max( 1.0f, CardClickScale ) : 1.0f;
	const FVector2D base = bUseManualCardSize ? ManualCardSize : LastAppliedCellSize_;
	if ( base.IsNearlyZero() )
	{
		return;
	}

	// Resize the SizeBox itself rather than render-scaling it: the underlying widget
	// re-rasterizes at the new dimensions, so text and icons stay sharp at any zoom.
	const FVector2D appliedSize = base * scale;
	sizeBox->SetWidthOverride( appliedSize.X );
	sizeBox->SetHeightOverride( appliedSize.Y );

	if ( UCanvasPanelSlot* slot = Cast<UCanvasPanelSlot>( sizeBox->Slot ) )
	{
		// Slot alignment is (0.5, 0.5), so resizing it grows the cell evenly around the cell
		// centre — neighbours don't shift, the card just visibly overlaps them.
		slot->SetSize( appliedSize );

		if ( bZoomed )
		{
			slot->SetZOrder( ZoomedCardZOrder );
		}
		else
		{
			// Restore the original Z-order — index of cell on its page.
			const int32 idx = CellSizeBoxes_.IndexOfByKey( sizeBox );
			if ( idx != INDEX_NONE && CellGridPositions_.IsValidIndex( idx ) )
			{
				const FIntPoint colRow = CellGridPositions_[idx];
				slot->SetZOrder( colRow.Y * GetCardsPerRow() + colRow.X );
			}
		}
	}
}

UCardSubsystem* UCardCollectionWidget::GetCardSubsystem()
{
	if ( CachedCardSubsystem_.IsValid() )
	{
		return CachedCardSubsystem_.Get();
	}

	CachedCardSubsystem_ = UCardSubsystem::Get( this );
	return CachedCardSubsystem_.Get();
}
