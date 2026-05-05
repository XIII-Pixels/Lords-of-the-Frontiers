#include "UI/Cards/CardCollectionWidget.h"

#include "Cards/CardDataAsset.h"
#include "Cards/CardSubsystem.h"
#include "Cards/CardTypes.h"
#include "UI/Cards/CardWidget.h"

#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/ScaleBox.h"
#include "Components/SizeBox.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"

void UCardCollectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( OpenBookButton )
	{
		OpenBookButton->OnClicked.AddDynamic( this, &UCardCollectionWidget::HandleOpenButtonClicked );
	}

	if ( CloseBookButton )
	{
		CloseBookButton->OnClicked.AddDynamic( this, &UCardCollectionWidget::HandleCloseButtonClicked );
	}

	if ( BackdropButton )
	{
		BackdropButton->OnClicked.AddDynamic( this, &UCardCollectionWidget::HandleBackdropClicked );
	}

	CloseAnimDelegate_.BindDynamic( this, &UCardCollectionWidget::HandleCloseAnimFinished );
	if ( BookCloseAnim )
	{
		BindToAnimationFinished( BookCloseAnim, CloseAnimDelegate_ );
	}

	SetBookVisualsVisible( false );
	bIsOpen_ = false;

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
}

void UCardCollectionWidget::NativeDestruct()
{
	if ( IsValid( OpenBookButton ) )
	{
		OpenBookButton->OnClicked.RemoveDynamic( this, &UCardCollectionWidget::HandleOpenButtonClicked );
	}

	if ( IsValid( CloseBookButton ) )
	{
		CloseBookButton->OnClicked.RemoveDynamic( this, &UCardCollectionWidget::HandleCloseButtonClicked );
	}

	if ( IsValid( BackdropButton ) )
	{
		BackdropButton->OnClicked.RemoveDynamic( this, &UCardCollectionWidget::HandleBackdropClicked );
	}

	if ( UCardSubsystem* subsystem = CachedCardSubsystem_.Get() )
	{
		subsystem->OnCardsApplied.RemoveDynamic( this, &UCardCollectionWidget::HandleCardsApplied );
	}

	if ( BookCloseAnim )
	{
		UnbindFromAnimationFinished( BookCloseAnim, CloseAnimDelegate_ );
	}

	ClearCells();

	Super::NativeDestruct();
}

void UCardCollectionWidget::RefreshFromSubsystem()
{
	UCardSubsystem* subsystem = GetCardSubsystem();
	if ( !subsystem )
	{
		RebuildCells( {} );
		return;
	}

	const TArray<UCardDataAsset*>& log = subsystem->GetAcquisitionLog();
	RebuildCells( BuildDisplayOrder( log ) );
}

void UCardCollectionWidget::SetCards( const TArray<UCardDataAsset*>& cards )
{
	RebuildCells( BuildDisplayOrder( cards ) );
}

void UCardCollectionWidget::OpenBook()
{
	if ( bIsOpen_ )
	{
		return;
	}

	bIsOpen_ = true;

	if ( bAutoRefreshOnOpen )
	{
		RefreshFromSubsystem();
	}

	SetBookVisualsVisible( true );

	if ( BookOpenAnim )
	{
		PlayAnimation( BookOpenAnim );
	}

	OnBookOpened();
}

void UCardCollectionWidget::CloseBook()
{
	if ( !bIsOpen_ )
	{
		return;
	}

	bIsOpen_ = false;

	if ( BookCloseAnim )
	{
		PlayAnimation( BookCloseAnim );
		// Final hide happens in HandleCloseAnimFinished.
		return;
	}

	SetBookVisualsVisible( false );
	OnBookClosed();
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

			lastIndexByCard[ card ] = insertAt;
		}
		else
		{
			lastIndexByCard.Add( card, result.Num() );
			result.Add( card );
		}
	}

	return result;
}

void UCardCollectionWidget::RebuildCells( const TArray<UCardDataAsset*>& displayOrder )
{
	ClearCells();

	if ( !LeftPageGrid || !RightPageGrid )
	{
		UE_LOG( LogTemp, Error,
			TEXT( "CardCollectionWidget: LeftPageGrid / RightPageGrid not bound in widget BP" ) );
		return;
	}

	const int32 perRow = GetCardsPerRow();
	const int32 rowsPerPage = GetRowsPerPage();
	const int32 cellsPerPage = perRow * rowsPerPage;
	const int32 totalCells = cellsPerPage * 2;

	for ( int32 cellIdx = 0; cellIdx < totalCells; ++cellIdx )
	{
		UCardDataAsset* cardData = displayOrder.IsValidIndex( cellIdx ) ? displayOrder[ cellIdx ] : nullptr;

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
		UPanelWidget* page = bRightPage ? RightPageGrid.Get() : LeftPageGrid.Get();
		const int32 indexOnPage = bRightPage ? cellIdx - cellsPerPage : cellIdx;

		AddCellToPage( page, sizeBox, indexOnPage );

		CellSizeBoxes_.Add( sizeBox );
		SpawnedCells_.Add( sizeBox );
	}

	// Fresh cells need an immediate auto-size pass so first paint isn't tiny.
	LastAppliedCellSize_ = FVector2D::ZeroVector;
	UpdateAutoCellSize();

	OnCollectionRebuilt( displayOrder.Num(), totalCells );
}

void UCardCollectionWidget::ClearCells()
{
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
	LastAppliedCellSize_ = FVector2D::ZeroVector;
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
		UE_LOG( LogTemp, Warning,
			TEXT( "CardCollectionWidget: no widget class for rarity %d and no fallback CardWidgetClass set" ),
			static_cast<int32>( cardData->Rarity ) );
		return nullptr;
	}

	UCardWidget* cardWidget = CreateWidget<UCardWidget>( this, widgetClass );
	if ( !cardWidget )
	{
		return nullptr;
	}

	cardWidget->SetCardData( cardData );
	// Cards in the book are read-only — selection is a reward-screen concept.
	cardWidget->SetInteractionEnabled( false );

	return cardWidget;
}

UWidget* UCardCollectionWidget::CreateEmptyCellWidget()
{
	if ( !QuestionSlotClass )
	{
		UE_LOG( LogTemp, Warning,
			TEXT( "CardCollectionWidget: QuestionSlotClass is not set — empty cells will be skipped" ) );
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

void UCardCollectionWidget::AddCellToPage( UPanelWidget* page, UWidget* cell, int32 cellIndexOnPage )
{
	if ( !page || !cell )
	{
		return;
	}

	if ( UUniformGridPanel* uniformGrid = Cast<UUniformGridPanel>( page ) )
	{
		const int32 perRow = GetCardsPerRow();
		const int32 row = cellIndexOnPage / perRow;
		const int32 col = cellIndexOnPage % perRow;

		if ( UUniformGridSlot* slot = uniformGrid->AddChildToUniformGrid( cell, row, col ) )
		{
			// Sizing is driven by the SizeBox wrapper, so the slot just centers it.
			slot->SetHorizontalAlignment( HAlign_Center );
			slot->SetVerticalAlignment( VAlign_Center );
		}
		return;
	}

	page->AddChild( cell );
}

void UCardCollectionWidget::SetBookVisualsVisible( bool bVisible )
{
	const ESlateVisibility shownVisibility = ESlateVisibility::SelfHitTestInvisible;
	const ESlateVisibility hiddenVisibility = ESlateVisibility::Collapsed;
	const ESlateVisibility target = bVisible ? shownVisibility : hiddenVisibility;

	if ( BookRoot )
	{
		BookRoot->SetVisibility( target );
	}

	// Hide every visual component independently so the cover, sheets and the
	// card/question grids all stay invisible until OpenBook is called, regardless
	// of whether the BP nests them inside BookRoot.
	if ( BookImage )
	{
		BookImage->SetVisibility( target );
	}

	if ( PagesImage )
	{
		PagesImage->SetVisibility( target );
	}

	if ( LeftPageGrid )
	{
		LeftPageGrid->SetVisibility( target );
	}

	if ( RightPageGrid )
	{
		RightPageGrid->SetVisibility( target );
	}

	// Backdrop only intercepts clicks while the book is open.
	if ( BackdropButton )
	{
		BackdropButton->SetVisibility( bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed );
	}
}

void UCardCollectionWidget::NativeTick( const FGeometry& myGeometry, float deltaTime )
{
	Super::NativeTick( myGeometry, deltaTime );

	// Auto sizing reacts to the actual rendered size of the page grids.
	// Only run while the book is on screen and using auto sizing — otherwise the
	// SizeBoxes already hold their final manual size.
	if ( !bIsOpen_ || bUseManualCardSize )
	{
		return;
	}

	UpdateAutoCellSize();
}

void UCardCollectionWidget::UpdateAutoCellSize()
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

	const FVector2D cellSize = ComputeAutoCellSize( pageSize );
	if ( cellSize.Equals( LastAppliedCellSize_, 0.5f ) )
	{
		return;
	}

	LastAppliedCellSize_ = cellSize;

	for ( const TObjectPtr<USizeBox>& sizeBox : CellSizeBoxes_ )
	{
		if ( !IsValid( sizeBox ) )
		{
			continue;
		}

		sizeBox->SetWidthOverride( cellSize.X );
		sizeBox->SetHeightOverride( cellSize.Y );
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
	OpenBook();
}

void UCardCollectionWidget::HandleCloseButtonClicked()
{
	CloseBook();
}

void UCardCollectionWidget::HandleBackdropClicked()
{
	CloseBook();
}

void UCardCollectionWidget::HandleCloseAnimFinished()
{
	if ( bIsOpen_ )
	{
		// A new OpenBook came in mid-close — keep it visible.
		return;
	}

	SetBookVisualsVisible( false );
	OnBookClosed();
}

void UCardCollectionWidget::HandleCardsApplied( const TArray<UCardDataAsset*>& appliedCards )
{
	if ( bIsOpen_ )
	{
		RefreshFromSubsystem();
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
