// GridEditorWidget.cpp

#if WITH_EDITOR

#include "Grid/GridEditorWidget.h"

#include "Building/Building.h"
#include "Grid/GridCell.h"
#include "Grid/GridManager.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/SpinBox.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

namespace GridColors
{

	static const FLinearColor Buildable( 0.15f, 0.75f, 0.3f, 1.0f );
	static const FLinearColor Blocked( 0.85f, 0.18f, 0.18f, 1.0f );
	static const FLinearColor Occupied( 0.2f, 0.45f, 0.95f, 1.0f );
	static const FLinearColor Selected( 1.0f, 0.85f, 0.0f, 1.0f );

	static const FLinearColor SectionBg( 0.12f, 0.12f, 0.14f, 1.0f );
	static const FLinearColor Separator( 0.25f, 0.25f, 0.28f, 1.0f );
	static const FLinearColor LabelColor( 0.75f, 0.75f, 0.78f, 1.0f );
	static const FLinearColor HeaderColor( 0.95f, 0.95f, 0.97f, 1.0f );
	static const FLinearColor ButtonBg( 0.22f, 0.22f, 0.26f, 1.0f );
} // namespace GridColors

void UGridCellClickHandler::OnClicked()
{
	if ( Owner )
	{
		Owner->SelectCell( X, Y );
	}
}

UTextBlock* UGridEditorWidget::MakeSectionHeader( const FString& text )
{
	UTextBlock* header = NewObject<UTextBlock>( this );
	header->SetText( FText::FromString( text ) );
	header->SetColorAndOpacity( FSlateColor( GridColors::HeaderColor ) );

	FSlateFontInfo font = header->GetFont();
	font.Size = 13;
	font.TypefaceFontName = FName( "Bold" );
	header->SetFont( font );

	return header;
}

UHorizontalBox* UGridEditorWidget::MakeLabeledRow( const FString& label, UWidget* valueWidget )
{
	UHorizontalBox* row = NewObject<UHorizontalBox>( this );

	UTextBlock* lbl = NewObject<UTextBlock>( this );
	lbl->SetText( FText::FromString( label ) );
	lbl->SetColorAndOpacity( FSlateColor( GridColors::LabelColor ) );

	FSlateFontInfo font = lbl->GetFont();
	font.Size = 11;
	lbl->SetFont( font );

	auto* lblSlot = row->AddChildToHorizontalBox( lbl );
	lblSlot->SetPadding( FMargin( 0.f, 0.f, 8.f, 0.f ) );
	lblSlot->SetVerticalAlignment( VAlign_Center );

	auto* valSlot = row->AddChildToHorizontalBox( valueWidget );
	valSlot->SetVerticalAlignment( VAlign_Center );

	return row;
}

UBorder* UGridEditorWidget::MakeSeparator()
{
	UBorder* sep = NewObject<UBorder>( this );
	sep->SetBrushColor( GridColors::Separator );

	USizeBox* sizeBox = NewObject<USizeBox>( this );
	sizeBox->SetHeightOverride( 1.0f );
	sep->AddChild( sizeBox );

	return sep;
}

void UGridEditorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( !GridManager_ )
	{
		FindGridManager();
	}

	BuildPropertiesPanel();
	BuildGrid();
}

void UGridEditorWidget::FindGridManager()
{
	GridManager_ = nullptr;

	TArray<UWorld*> worldsToSearch;

	if ( GEditor )
	{
		UWorld* editorWorld = GEditor->GetEditorWorldContext().World();
		if ( editorWorld )
		{
			worldsToSearch.AddUnique( editorWorld );
		}

		for ( const FWorldContext& context : GEngine->GetWorldContexts() )
		{
			if ( context.World() )
			{
				worldsToSearch.AddUnique( context.World() );
			}
		}
	}

	for ( UWorld* world : worldsToSearch )
	{
		for ( TActorIterator<AGridManager> it( world ); it; ++it )
		{
			GridManager_ = *it;
			UE_LOG(
			    LogTemp, Log, TEXT( "GridEditorWidget: Found GridManager '%s' in world '%s'" ),
			    *GridManager_->GetName(), *world->GetName()
			);
			return;
		}
	}

	UE_LOG( LogTemp, Warning, TEXT( "GridEditorWidget: GridManager not found in any world" ) );
}

void UGridEditorWidget::RefreshGrid()
{
	RefreshBuildingList();
	BuildGrid();
}

void UGridEditorWidget::SelectCell( const int32 x, const int32 y )
{
	const FIntPoint prevSelected = SelectedCoords_;
	SelectedCoords_ = FIntPoint( x, y );

	if ( GridManager_ && GridManager_->IsValidCoords( prevSelected.X, prevSelected.Y ) )
	{
		UpdateCellColor( prevSelected.X, prevSelected.Y );
	}

	UpdateCellColor( x, y );
	UpdatePropertiesPanel();
}

void UGridEditorWidget::RefreshBuildingList()
{
	BuildingMap_.Empty();

	if ( !OccupantComboBox_ )
	{
		return;
	}

	bUpdatingUI_ = true;

	OccupantComboBox_->ClearOptions();
	OccupantComboBox_->AddOption( TEXT( "\u2014 \u041D\u0435\u0442 \u2014" ) );

	TSet<ABuilding*> assignedBuildings;

	if ( GridManager_ )
	{
		const int32 height = GridManager_->GetGridHeight();

		for ( int32 y = 0; y < height; ++y )
		{
			const int32 rowWidth = GridManager_->GetRowWidth( y );

			for ( int32 x = 0; x < rowWidth; ++x )
			{
				const FGridCell* cell = GridManager_->GetCell( x, y );
				if ( cell && cell->Occupant.IsValid() )
				{
					assignedBuildings.Add( cell->Occupant.Get() );
				}
			}
		}
	}

	ABuilding* selectedCellOccupant = nullptr;

	if ( GridManager_ && SelectedCoords_.X >= 0 && SelectedCoords_.Y >= 0 )
	{
		const FGridCell* selectedCell = GridManager_->GetCell( SelectedCoords_.X, SelectedCoords_.Y );

		if ( selectedCell && selectedCell->Occupant.IsValid() )
		{
			selectedCellOccupant = selectedCell->Occupant.Get();
		}
	}

	TArray<UWorld*> worldsToSearch;

	if ( GEditor )
	{
		UWorld* editorWorld = GEditor->GetEditorWorldContext().World();
		if ( editorWorld )
		{
			worldsToSearch.AddUnique( editorWorld );
		}

		for ( const FWorldContext& context : GEngine->GetWorldContexts() )
		{
			if ( context.World() )
			{
				worldsToSearch.AddUnique( context.World() );
			}
		}
	}

	for ( UWorld* world : worldsToSearch )
	{
		for ( TActorIterator<ABuilding> it( world ); it; ++it )
		{
			ABuilding* building = *it;
			if ( !building )
			{
				continue;
			}

			if ( building->HasAnyFlags( RF_Transient ) || building->bIsEditorPreviewActor )
			{
				continue;
			}

			if ( assignedBuildings.Contains( building ) && building != selectedCellOccupant )
			{
				continue;
			}

			FString displayName = building->GetActorNameOrLabel();

			if ( BuildingMap_.Contains( displayName ) )
			{
				displayName = FString::Printf( TEXT( "%s [%s]" ), *displayName, *building->GetName() );
			}

			BuildingMap_.Add( displayName, building );
			OccupantComboBox_->AddOption( displayName );
		}
	}

	UE_LOG( LogTemp, Log, TEXT( "GridEditorWidget: Found %d available buildings" ), BuildingMap_.Num() );

	bUpdatingUI_ = false;
}

void UGridEditorWidget::BuildGrid()
{
	if ( !GridScrollBox_ )
	{
		UE_LOG( LogTemp, Warning, TEXT( "GridEditorWidget: GridScrollBox_ is null (BindWidget missing?)" ) );
		return;
	}

	GridScrollBox_->ClearChildren();
	CellBorderMap_.Empty();
	ClickHandlers_.Empty();
	GridPanel_ = nullptr;

	if ( !GridManager_ )
	{
		UTextBlock* hint = NewObject<UTextBlock>( this );
		hint->SetText(
		    FText::FromString( TEXT(
		        "GridManager \u043D\u0435 \u043D\u0430\u0439\u0434\u0435\u043D.\n"
		        "\u041D\u0430\u0436\u043C\u0438\u0442\u0435 \u00AB\u041E\u0431\u043D\u043E\u0432"
		        "\u0438\u0442\u044C\u00BB \u0438\u043B\u0438 \u043D\u0430\u0437\u043D\u0430\u0447"
		        "\u044C\u0442\u0435 \u0432\u0440\u0443\u0447\u043D\u0443\u044E."
		    ) )
		);
		hint->SetColorAndOpacity( FSlateColor( GridColors::LabelColor ) );
		GridScrollBox_->AddChild( hint );
		return;
	}

	const int32 height = GridManager_->GetGridHeight();
	const int32 maxWidth = GridManager_->GetMaxWidth();

	if ( height <= 0 || maxWidth <= 0 )
	{
		UTextBlock* hint = NewObject<UTextBlock>( this );
		hint->SetText(
		    FText::FromString( TEXT( "\u0421\u0435\u0442\u043A\u0430 \u043F\u0443\u0441\u0442\u0430 (0\u00D70)." ) )
		);
		hint->SetColorAndOpacity( FSlateColor( GridColors::LabelColor ) );
		GridScrollBox_->AddChild( hint );
		return;
	}

	UTextBlock* sizeLabel = NewObject<UTextBlock>( this );
	sizeLabel->SetText(
	    FText::FromString(
	        FString::Printf(
	            TEXT(
	                "\u0421\u0435\u0442\u043A\u0430: %d \u0441\u0442\u0440\u043E\u043A, "
	                "\u043C\u0430\u043A\u0441. \u0448\u0438\u0440\u0438\u043D\u0430 %d  |  "
	                "\u0420\u0430\u0437\u043C\u0435\u0440 \u043A\u043B\u0435\u0442\u043A"
	                "\u0438: %.0f"
	            ),
	            height, maxWidth, GridManager_->GetCellSize()
	        )
	    )
	);
	sizeLabel->SetColorAndOpacity( FSlateColor( GridColors::LabelColor ) );

	FSlateFontInfo infoFont = sizeLabel->GetFont();
	infoFont.Size = 10;
	sizeLabel->SetFont( infoFont );

	GridScrollBox_->AddChild( sizeLabel );

	GridPanel_ = NewObject<UUniformGridPanel>( this );
	GridPanel_->SetSlotPadding( FMargin( 1.5f ) );
	GridScrollBox_->AddChild( GridPanel_ );

	for ( int32 y = 0; y < height; ++y )
	{
		const int32 rowWidth = GridManager_->GetRowWidth( y );

		for ( int32 x = 0; x < rowWidth; ++x )
		{
			USizeBox* cellSizeBox = NewObject<USizeBox>( this );
			cellSizeBox->SetWidthOverride( CellButtonSize_ );
			cellSizeBox->SetHeightOverride( CellButtonSize_ );

			UBorder* border = NewObject<UBorder>( this );
			border->SetPadding( FMargin( 2.0f ) );
			cellSizeBox->AddChild( border );

			UButton* button = NewObject<UButton>( this );
			button->SetBackgroundColor( FLinearColor( 0.0f, 0.0f, 0.0f, 0.3f ) );
			border->AddChild( button );

			UTextBlock* label = NewObject<UTextBlock>( this );
			label->SetText( FText::FromString( FString::Printf( TEXT( "%d,%d" ), x, y ) ) );

			FSlateFontInfo fontInfo = label->GetFont();
			fontInfo.Size = 7;
			label->SetFont( fontInfo );
			label->SetJustification( ETextJustify::Center );
			label->SetColorAndOpacity( FSlateColor( FLinearColor( 1.f, 1.f, 1.f, 0.85f ) ) );
			button->AddChild( label );

			UGridCellClickHandler* handler = NewObject<UGridCellClickHandler>( this );
			handler->X = x;
			handler->Y = y;
			handler->Owner = this;
			ClickHandlers_.Add( handler );

			button->OnClicked.AddDynamic( handler, &UGridCellClickHandler::OnClicked );

			GridPanel_->AddChildToUniformGrid( cellSizeBox, y, x );
			CellBorderMap_.Add( FIntPoint( x, y ), border );
		}

		for ( int32 x = rowWidth; x < maxWidth; ++x )
		{
			USizeBox* emptyBox = NewObject<USizeBox>( this );
			emptyBox->SetWidthOverride( CellButtonSize_ );
			emptyBox->SetHeightOverride( CellButtonSize_ );
			GridPanel_->AddChildToUniformGrid( emptyBox, y, x );
		}
	}

	UpdateAllCellColors();
	RefreshBuildingList();

	SelectedCoords_ = FIntPoint( -1, -1 );
	UpdatePropertiesPanel();
}

void UGridEditorWidget::BuildPropertiesPanel()
{
	if ( !PropertiesPanel_ )
	{
		return;
	}

	PropertiesPanel_->ClearChildren();

	{
		UBorder* titleBg = NewObject<UBorder>( this );
		titleBg->SetBrushColor( GridColors::SectionBg );
		titleBg->SetPadding( FMargin( 10.f, 8.f ) );

		UTextBlock* title = NewObject<UTextBlock>( this );
		title->SetText(
		    FText::FromString( TEXT(
		        "\u2699  \u0420\u0435\u0434\u0430\u043A\u0442\u043E\u0440 "
		        "\u0441\u0435\u0442\u043A\u0438"
		    ) )
		);
		title->SetColorAndOpacity( FSlateColor( GridColors::HeaderColor ) );

		FSlateFontInfo titleFont = title->GetFont();
		titleFont.Size = 16;
		titleFont.TypefaceFontName = FName( "Bold" );
		title->SetFont( titleFont );

		titleBg->AddChild( title );
		PropertiesPanel_->AddChild( titleBg );
	}

	{
		RefreshButton_ = NewObject<UButton>( this );
		RefreshButton_->SetBackgroundColor( GridColors::ButtonBg );
		RefreshButton_->OnClicked.AddDynamic( this, &UGridEditorWidget::OnRefreshClicked );

		UTextBlock* btnText = NewObject<UTextBlock>( this );
		btnText->SetText( FText::FromString( TEXT( "\U0001F504  \u041E\u0431\u043D\u043E\u0432\u0438\u0442\u044C" ) ) );
		btnText->SetColorAndOpacity( FSlateColor( GridColors::HeaderColor ) );

		FSlateFontInfo btnFont = btnText->GetFont();
		btnFont.Size = 11;
		btnText->SetFont( btnFont );

		RefreshButton_->AddChild( btnText );

		auto* btnSlot = PropertiesPanel_->AddChildToVerticalBox( RefreshButton_ );
		btnSlot->SetPadding( FMargin( 8.f, 6.f, 8.f, 2.f ) );
	}

	PropertiesPanel_->AddChild( MakeSeparator() );

	CoordsText_ = NewObject<UTextBlock>( this );
	CoordsText_->SetText(
	    FText::FromString( TEXT(
	        "\u0412\u044B\u0431\u0435\u0440\u0438\u0442\u0435 \u043A\u043B\u0435\u0442\u043A\u0443 "
	        "\u043D\u0430 \u0441\u0435\u0442\u043A\u0435"
	    ) )
	);
	CoordsText_->SetColorAndOpacity( FSlateColor( GridColors::HeaderColor ) );
	{
		FSlateFontInfo font = CoordsText_->GetFont();
		font.Size = 12;
		CoordsText_->SetFont( font );
	}

	auto* coordsSlot = PropertiesPanel_->AddChildToVerticalBox( CoordsText_ );
	coordsSlot->SetPadding( FMargin( 8.f, 8.f, 8.f, 4.f ) );

	PropertiesPanel_->AddChild( MakeSeparator() );

	{
		auto* headerSlot = PropertiesPanel_->AddChildToVerticalBox( MakeSectionHeader( TEXT(
		    "\u0421\u0432\u043E\u0439\u0441\u0442\u0432\u0430 "
		    "\u043A\u043B\u0435\u0442\u043A\u0438"
		) ) );
		headerSlot->SetPadding( FMargin( 8.f, 8.f, 8.f, 4.f ) );
	}

	{
		BuildableCheck_ = NewObject<UCheckBox>( this );
		BuildableCheck_->OnCheckStateChanged.AddDynamic( this, &UGridEditorWidget::OnBuildableChanged );

		UHorizontalBox* row = MakeLabeledRow(
		    TEXT( "\u041C\u043E\u0436\u043D\u043E \u0441\u0442\u0440\u043E\u0438\u0442\u044C:" ), BuildableCheck_
		);

		auto* rowSlot = PropertiesPanel_->AddChildToVerticalBox( row );
		rowSlot->SetPadding( FMargin( 12.f, 4.f ) );
	}

	{
		WalkableCheck_ = NewObject<UCheckBox>( this );
		WalkableCheck_->OnCheckStateChanged.AddDynamic( this, &UGridEditorWidget::OnWalkableChanged );

		UHorizontalBox* row =
		    MakeLabeledRow( TEXT( "\u041F\u0440\u043E\u0445\u043E\u0434\u0438\u043C\u0430\u044F:" ), WalkableCheck_ );
		auto* rowSlot = PropertiesPanel_->AddChildToVerticalBox( row );
		rowSlot->SetPadding( FMargin( 12.f, 4.f ) );
	}

	{
		BonusSpinBox_ = NewObject<USpinBox>( this );
		BonusSpinBox_->SetMinValue( 0.0f );
		BonusSpinBox_->SetMaxValue( 100.0f );
		BonusSpinBox_->SetDelta( 0.1f );
		BonusSpinBox_->OnValueChanged.AddDynamic( this, &UGridEditorWidget::OnBonusValueChanged );

		UHorizontalBox* row = MakeLabeledRow( TEXT( "\u0411\u043E\u043D\u0443\u0441:" ), BonusSpinBox_ );
		auto* rowSlot = PropertiesPanel_->AddChildToVerticalBox( row );
		rowSlot->SetPadding( FMargin( 12.f, 4.f ) );
	}

	PropertiesPanel_->AddChild( MakeSeparator() );

	{
		auto* headerSlot = PropertiesPanel_->AddChildToVerticalBox( MakeSectionHeader( TEXT(
		    "\u0417\u0434\u0430\u043D\u0438\u0435 \u0432 "
		    "\u043A\u043B\u0435\u0442\u043A\u0435"
		) ) );
		headerSlot->SetPadding( FMargin( 8.f, 8.f, 8.f, 4.f ) );
	}

	OccupantText_ = NewObject<UTextBlock>( this );
	OccupantText_->SetText(
	    FText::FromString( TEXT( "\u0422\u0435\u043A\u0443\u0449\u0435\u0435: \u043D\u0435\u0442" ) )
	);
	OccupantText_->SetColorAndOpacity( FSlateColor( GridColors::LabelColor ) );
	{
		FSlateFontInfo font = OccupantText_->GetFont();
		font.Size = 11;
		OccupantText_->SetFont( font );

		auto* occSlot = PropertiesPanel_->AddChildToVerticalBox( OccupantText_ );
		occSlot->SetPadding( FMargin( 12.f, 4.f ) );
	}

	{
		OccupantComboBox_ = NewObject<UComboBoxString>( this );
		OccupantComboBox_->AddOption( TEXT( "\u2014 \u041D\u0435\u0442 \u2014" ) );
		OccupantComboBox_->OnSelectionChanged.AddDynamic( this, &UGridEditorWidget::OnOccupantSelected );

		UHorizontalBox* row =
		    MakeLabeledRow( TEXT( "\u041D\u0430\u0437\u043D\u0430\u0447\u0438\u0442\u044C:" ), OccupantComboBox_ );
		auto* rowSlot = PropertiesPanel_->AddChildToVerticalBox( row );
		rowSlot->SetPadding( FMargin( 12.f, 4.f ) );
	}

	{
		ClearOccupantButton_ = NewObject<UButton>( this );
		ClearOccupantButton_->SetBackgroundColor( FLinearColor( 0.6f, 0.15f, 0.15f, 1.0f ) );
		ClearOccupantButton_->OnClicked.AddDynamic( this, &UGridEditorWidget::OnClearOccupantClicked );

		UTextBlock* btnLabel = NewObject<UTextBlock>( this );
		btnLabel->SetText(
		    FText::FromString( TEXT(
		        "\u2715  \u0423\u0431\u0440\u0430\u0442\u044C "
		        "\u0437\u0434\u0430\u043D\u0438\u0435"
		    ) )
		);
		btnLabel->SetColorAndOpacity( FSlateColor( FLinearColor::White ) );

		FSlateFontInfo btnFont = btnLabel->GetFont();
		btnFont.Size = 10;
		btnLabel->SetFont( btnFont );

		ClearOccupantButton_->AddChild( btnLabel );

		auto* btnSlot = PropertiesPanel_->AddChildToVerticalBox( ClearOccupantButton_ );
		btnSlot->SetPadding( FMargin( 12.f, 6.f, 12.f, 2.f ) );
	}

	PropertiesPanel_->AddChild( MakeSeparator() );

	{
		auto* headerSlot = PropertiesPanel_->AddChildToVerticalBox(
		    MakeSectionHeader( TEXT( "\u041B\u0435\u0433\u0435\u043D\u0434\u0430" ) )
		);
		headerSlot->SetPadding( FMargin( 8.f, 8.f, 8.f, 2.f ) );
	}

	auto addLegendItem = [this]( const FString& text, const FLinearColor& color )
	{
		UHorizontalBox* row = NewObject<UHorizontalBox>( this );

		UBorder* swatch = NewObject<UBorder>( this );
		swatch->SetBrushColor( color );

		USizeBox* swatchSize = NewObject<USizeBox>( this );
		swatchSize->SetWidthOverride( 14.0f );
		swatchSize->SetHeightOverride( 14.0f );
		swatchSize->AddChild( swatch );

		auto* swatchSlot = row->AddChildToHorizontalBox( swatchSize );
		swatchSlot->SetPadding( FMargin( 0.f, 0.f, 6.f, 0.f ) );
		swatchSlot->SetVerticalAlignment( VAlign_Center );

		UTextBlock* lbl = NewObject<UTextBlock>( this );
		lbl->SetText( FText::FromString( text ) );
		lbl->SetColorAndOpacity( FSlateColor( GridColors::LabelColor ) );

		FSlateFontInfo font = lbl->GetFont();
		font.Size = 10;
		lbl->SetFont( font );

		auto* lblSlot = row->AddChildToHorizontalBox( lbl );
		lblSlot->SetVerticalAlignment( VAlign_Center );

		auto* rowSlot = PropertiesPanel_->AddChildToVerticalBox( row );
		rowSlot->SetPadding( FMargin( 12.f, 2.f ) );
	};

	addLegendItem(
	    TEXT( "\u041C\u043E\u0436\u043D\u043E \u0441\u0442\u0440\u043E\u0438\u0442\u044C" ), GridColors::Buildable
	);
	addLegendItem(
	    TEXT( "\u0417\u0430\u0431\u043B\u043E\u043A\u0438\u0440\u043E\u0432\u0430\u043D\u0430" ), GridColors::Blocked
	);
	addLegendItem(
	    TEXT( "\u0417\u0430\u043D\u044F\u0442\u0430 \u0437\u0434\u0430\u043D\u0438\u0435\u043C" ), GridColors::Occupied
	);
	addLegendItem( TEXT( "\u0412\u044B\u0431\u0440\u0430\u043D\u0430" ), GridColors::Selected );
}

FLinearColor UGridEditorWidget::GetColorForCell( const FGridCell& cell, const bool bIsSelected ) const
{
	if ( bIsSelected )
	{
		return GridColors::Selected;
	}

	if ( cell.bIsOccupied || cell.Occupant.IsValid() )
	{
		return GridColors::Occupied;
	}

	if ( !cell.bIsBuildable )
	{
		return GridColors::Blocked;
	}

	return GridColors::Buildable;
}

void UGridEditorWidget::UpdateCellColor( const int32 x, const int32 y )
{
	if ( !GridManager_ )
	{
		return;
	}

	const FGridCell* cell = GridManager_->GetCell( x, y );
	if ( !cell )
	{
		return;
	}

	const FIntPoint key( x, y );
	TObjectPtr<UBorder>* borderPtr = CellBorderMap_.Find( key );
	if ( !borderPtr || !( *borderPtr ) )
	{
		return;
	}

	const bool bSelected = ( SelectedCoords_.X == x && SelectedCoords_.Y == y );
	( *borderPtr )->SetBrushColor( GetColorForCell( *cell, bSelected ) );
}

void UGridEditorWidget::UpdateAllCellColors()
{
	if ( !GridManager_ )
	{
		return;
	}

	const int32 height = GridManager_->GetGridHeight();

	for ( int32 y = 0; y < height; ++y )
	{
		const int32 rowWidth = GridManager_->GetRowWidth( y );

		for ( int32 x = 0; x < rowWidth; ++x )
		{
			UpdateCellColor( x, y );
		}
	}
}

void UGridEditorWidget::UpdatePropertiesPanel()
{
	bUpdatingUI_ = true;
	const FString noneOption = TEXT( "\u2014 \u041D\u0435\u0442 \u2014" );

	if ( !GridManager_ || SelectedCoords_.X < 0 || SelectedCoords_.Y < 0 )
	{
		if ( CoordsText_ )
		{
			CoordsText_->SetText(
			    FText::FromString( TEXT(
			        "\u0412\u044B\u0431\u0435\u0440\u0438\u0442\u0435 \u043A\u043B\u0435\u0442"
			        "\u043A\u0443 \u043D\u0430 \u0441\u0435\u0442\u043A\u0435"
			    ) )
			);
		}
		if ( OccupantText_ )
		{
			OccupantText_->SetText(
			    FText::FromString( TEXT( "\u0422\u0435\u043A\u0443\u0449\u0435\u0435: \u043D\u0435\u0442" ) )
			);
		}
		if ( OccupantComboBox_ )
		{
			OccupantComboBox_->SetSelectedOption( noneOption );
		}
		return;
	}

	const FGridCell* cell = GridManager_->GetCell( SelectedCoords_.X, SelectedCoords_.Y );
	if ( !cell )
	{
		bUpdatingUI_ = false;
		return;
	}

	if ( CoordsText_ )
	{
		CoordsText_->SetText(
		    FText::FromString(
		        FString::Printf(
		            TEXT( "\U0001F4CD  \u041A\u043B\u0435\u0442\u043A\u0430 (%d, %d)" ), SelectedCoords_.X,
		            SelectedCoords_.Y
		        )
		    )
		);
	}

	if ( BuildableCheck_ )
	{
		BuildableCheck_->SetIsChecked( cell->bIsBuildable );
	}

	if ( WalkableCheck_ )
	{
		WalkableCheck_->SetIsChecked( cell->bIsWalkable );
	}

	if ( BonusSpinBox_ )
	{
		BonusSpinBox_->SetValue( cell->BuildBonus );
	}

	if ( OccupantText_ )
	{
		if ( cell->Occupant.IsValid() )
		{
			OccupantText_->SetText(
			    FText::FromString(
			        FString::Printf(
			            TEXT( "\u0422\u0435\u043A\u0443\u0449\u0435\u0435: %s" ), *cell->Occupant->GetActorNameOrLabel()
			        )
			    )
			);
		}
		else
		{
			OccupantText_->SetText(
			    FText::FromString( TEXT( "\u0422\u0435\u043A\u0443\u0449\u0435\u0435: \u043D\u0435\u0442" ) )
			);
		}
	}

	if ( OccupantComboBox_ )
	{
		if ( cell->Occupant.IsValid() )
		{
			FString foundName = noneOption;
			for ( const auto& pair : BuildingMap_ )
			{
				if ( pair.Value == cell->Occupant.Get() )
				{
					foundName = pair.Key;
					break;
				}
			}
			OccupantComboBox_->SetSelectedOption( foundName );
		}
		else
		{
			OccupantComboBox_->SetSelectedOption( noneOption );
		}
	}
	bUpdatingUI_ = false;
}

void UGridEditorWidget::OnBuildableChanged( const bool bIsChecked )
{
	if ( !GridManager_ || SelectedCoords_.X < 0 )
	{
		return;
	}

	FGridCell* cell = GridManager_->GetCell( SelectedCoords_.X, SelectedCoords_.Y );
	if ( cell )
	{
		cell->bIsBuildable = bIsChecked;
		GridManager_->MarkPackageDirty();
		UpdateCellColor( SelectedCoords_.X, SelectedCoords_.Y );
	}
}

void UGridEditorWidget::OnWalkableChanged( const bool bIsChecked )
{
	if ( !GridManager_ || SelectedCoords_.X < 0 )
	{
		return;
	}

	FGridCell* cell = GridManager_->GetCell( SelectedCoords_.X, SelectedCoords_.Y );
	if ( cell )
	{
		cell->bIsWalkable = bIsChecked;
		GridManager_->MarkPackageDirty();
		UpdateCellColor( SelectedCoords_.X, SelectedCoords_.Y );
	}
}

void UGridEditorWidget::OnBonusValueChanged( const float inValue )
{
	if ( !GridManager_ || SelectedCoords_.X < 0 )
	{
		return;
	}

	FGridCell* cell = GridManager_->GetCell( SelectedCoords_.X, SelectedCoords_.Y );
	if ( cell )
	{
		cell->BuildBonus = inValue;
		GridManager_->MarkPackageDirty();
	}
}

void UGridEditorWidget::OnOccupantSelected( FString selectedItem, ESelectInfo::Type selectionType )
{
	if ( bUpdatingUI_ )
	{
		return;
	}
	const FString noneOption = TEXT( "\u2014 \u041D\u0435\u0442 \u2014" );

	if ( !GridManager_ || SelectedCoords_.X < 0 || SelectedCoords_.Y < 0 )
	{
		return;
	}

	FGridCell* cell = GridManager_->GetCell( SelectedCoords_.X, SelectedCoords_.Y );
	if ( !cell )
	{
		return;
	}

	if ( selectedItem == noneOption || selectedItem.IsEmpty() )
	{
		cell->Occupant = nullptr;
		cell->bIsOccupied = false;
	}
	else
	{
		TObjectPtr<ABuilding>* foundBuilding = BuildingMap_.Find( selectedItem );
		if ( foundBuilding && *foundBuilding )
		{
			cell->Occupant = *foundBuilding;
			cell->bIsOccupied = true;

			FVector center;
			if ( GridManager_->GetCellWorldCenter( cell->GridCoords, center ) )
			{
				( *foundBuilding )->SetActorLocation( center );
			}
		}
	}

	GridManager_->MarkPackageDirty();
	UpdateCellColor( SelectedCoords_.X, SelectedCoords_.Y );
	RefreshBuildingList();
	UpdatePropertiesPanel();
}

void UGridEditorWidget::OnClearOccupantClicked()
{
	if ( !GridManager_ || SelectedCoords_.X < 0 || SelectedCoords_.Y < 0 )
	{
		return;
	}

	FGridCell* cell = GridManager_->GetCell( SelectedCoords_.X, SelectedCoords_.Y );
	if ( !cell )
	{
		return;
	}

	cell->Occupant = nullptr;
	cell->bIsOccupied = false;

	GridManager_->MarkPackageDirty();
	UpdateCellColor( SelectedCoords_.X, SelectedCoords_.Y );
	RefreshBuildingList();
	UpdatePropertiesPanel();
}

void UGridEditorWidget::OnRefreshClicked()
{
	if ( !GridManager_ )
	{
		FindGridManager();
	}

	RefreshGrid();
}

#endif