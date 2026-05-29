#include "UI/InfoWaves/WaveInfoPanelWidget.h"

#include "UI/InfoWaves/EnemyInfoDataAsset.h"
#include "UI/InfoWaves/EnemyRowWidget.h"
#include "Units/Unit.h"

#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/SizeBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/AudioTags.h"
#include "Sound/SoundEffectManager.h"

void UWaveInfoPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( IsValid( ButtonToggle ) )
	{
		ButtonToggle->OnClicked.AddDynamic( this, &UWaveInfoPanelWidget::OnButtonToggleClicked );
		ButtonToggle->OnHovered.AddDynamic( this, &UWaveInfoPanelWidget::OnButtonToggleHovered );
		ButtonToggle->OnUnhovered.AddDynamic( this, &UWaveInfoPanelWidget::OnButtonToggleUnhovered );
	}

	bIsOpen_ = bStartOpen_;
	bButtonHovered_ = false;

	SetVisibility( ESlateVisibility::SelfHitTestInvisible );

	ApplyBottomAlignment();
	RefreshSlideDistance();
	CurrentBaseY_ = ComputeBaseTargetY();
	CurrentHoverY_ = ComputeHoverTargetY();
	CurrentY_ = CurrentBaseY_ + CurrentHoverY_;
	ApplyCurrentOffset();
	ApplyButtonSize();

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

void UWaveInfoPanelWidget::NativeDestruct()
{
	if ( IsValid( ButtonToggle ) )
	{
		ButtonToggle->OnClicked.RemoveDynamic( this, &UWaveInfoPanelWidget::OnButtonToggleClicked );
		ButtonToggle->OnHovered.RemoveDynamic( this, &UWaveInfoPanelWidget::OnButtonToggleHovered );
		ButtonToggle->OnUnhovered.RemoveDynamic( this, &UWaveInfoPanelWidget::OnButtonToggleUnhovered );
	}

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

void UWaveInfoPanelWidget::NativeTick( const FGeometry& myGeometry, float inDeltaTime )
{
	Super::NativeTick( myGeometry, inDeltaTime );

	const float baseTarget = ComputeBaseTargetY();
	const float hoverTarget = ComputeHoverTargetY();

	const bool bNeedsBase = !FMath::IsNearlyEqual( CurrentBaseY_, baseTarget );
	const bool bNeedsHover = !FMath::IsNearlyEqual( CurrentHoverY_, hoverTarget );

	if ( !bNeedsBase && !bNeedsHover )
	{
		return;
	}

	if ( bNeedsBase )
	{
		const float baseSpan = FMath::Max( CachedSlideDistance_, 1.0f );
		const float baseSpeed = ( AnimDuration_ > 0.0f ) ? ( baseSpan / AnimDuration_ ) : 1000.0f;
		CurrentBaseY_ = FMath::FInterpConstantTo( CurrentBaseY_, baseTarget, inDeltaTime, baseSpeed );
	}

	if ( bNeedsHover )
	{
		const float hoverSpan = FMath::Max( HoverPeekDistance_, 1.0f );
		const float hoverSpeed = ( HoverAnimDuration_ > 0.0f ) ? ( hoverSpan / HoverAnimDuration_ ) : 1000.0f;
		CurrentHoverY_ = FMath::FInterpConstantTo( CurrentHoverY_, hoverTarget, inDeltaTime, hoverSpeed );
	}

	CurrentY_ = CurrentBaseY_ + CurrentHoverY_;
	ApplyCurrentOffset();
	ApplyButtonSize();
}

float UWaveInfoPanelWidget::ComputeBaseTargetY() const
{
	return bIsOpen_ ? CachedSlideDistance_ : 0.0f;
}

float UWaveInfoPanelWidget::ComputeHoverTargetY() const
{
	if ( !bButtonHovered_ )
	{
		return 0.0f;
	}
	return ( bIsOpen_ ? -1.0f : 1.0f ) * HoverPeekDistance_;
}

void UWaveInfoPanelWidget::ApplyCurrentOffset()
{
	if ( !IsValid( SlideContent ) )
	{
		return;
	}
	SlideContent->SetRenderTranslation( FVector2D( 0.0f, CurrentY_ ) );
}

void UWaveInfoPanelWidget::ApplyButtonSize()
{
	if ( !IsValid( ButtonToggle ) )
	{
		return;
	}

	UCanvasPanelSlot* slot = Cast<UCanvasPanelSlot>( ButtonToggle->Slot );
	if ( !slot )
	{
		return;
	}

	FVector2D size = ButtonClosedSize_;
	size.Y += FMath::Max( 0.0f, CurrentBaseY_ );

	slot->SetSize( size );
}

void UWaveInfoPanelWidget::RefreshSlideDistance()
{
	if ( !IsValid( EnemyListContainer ) )
	{
		CachedSlideDistance_ = FallbackSlideDistance_;
		return;
	}

	EnemyListContainer->ForceLayoutPrepass();
	float h = EnemyListContainer->GetDesiredSize().Y;
	if ( h < 1.0f )
	{
		h = FallbackSlideDistance_;
	}
	else
	{
		h += SlidePadding_;
	}

	CachedSlideDistance_ = FMath::Max( h, MinSlideDistance_ );
}

void UWaveInfoPanelWidget::ApplyBottomAlignment()
{
	if ( !bFillBottomUp_ || !IsValid( EnemyListContainer ) || !EnemyListContainer->Slot )
	{
		return;
	}

	UPanelSlot* slot = EnemyListContainer->Slot;
	const FMargin bottomPad( 0.0f, 0.0f, 0.0f, EnemyListBottomPadding_ );

	if ( UOverlaySlot* overlaySlot = Cast<UOverlaySlot>( slot ) )
	{
		overlaySlot->SetHorizontalAlignment( HAlign_Center );
		overlaySlot->SetVerticalAlignment( VAlign_Bottom );
		overlaySlot->SetPadding( bottomPad );
	}
	else if ( USizeBoxSlot* sizeBoxSlot = Cast<USizeBoxSlot>( slot ) )
	{
		sizeBoxSlot->SetHorizontalAlignment( HAlign_Center );
		sizeBoxSlot->SetVerticalAlignment( VAlign_Bottom );
		sizeBoxSlot->SetPadding( bottomPad );
	}
	else if ( UBorderSlot* borderSlot = Cast<UBorderSlot>( slot ) )
	{
		borderSlot->SetHorizontalAlignment( HAlign_Center );
		borderSlot->SetVerticalAlignment( VAlign_Bottom );
		borderSlot->SetPadding( bottomPad );
	}
	else if ( UCanvasPanelSlot* canvasSlot = Cast<UCanvasPanelSlot>( slot ) )
	{
		FAnchors anchors;
		anchors.Minimum = FVector2D( 0.5f, 1.0f );
		anchors.Maximum = FVector2D( 0.5f, 1.0f );
		canvasSlot->SetAnchors( anchors );
		canvasSlot->SetAlignment( FVector2D( 0.5f, 1.0f ) );
		canvasSlot->SetPosition( FVector2D( 0.0f, -EnemyListBottomPadding_ ) );
		canvasSlot->SetAutoSize( true );
	}
}

void UWaveInfoPanelWidget::PopulatePanel( const TMap<TSubclassOf<AUnit>, int32>& waveData )
{
	if ( !IsValid( EnemyListContainer ) || !IsValid( EnemyRowClass ) )
	{
		UE_LOG( LogTemp, Warning,
		        TEXT( "WaveInfoPanel: PopulatePanel skipped — EnemyListContainer=%s EnemyRowClass=%s" ),
		        IsValid( EnemyListContainer ) ? TEXT( "ok" ) : TEXT( "null" ),
		        IsValid( EnemyRowClass ) ? TEXT( "ok" ) : TEXT( "null" ) );
		return;
	}

	if ( !IsValid( EnemyDataAsset ) )
	{
		UE_LOG( LogTemp, Warning, TEXT( "WaveInfoPanel: EnemyDataAsset is null — rows will not be created" ) );
	}

	int32 totalEnemies = 0;
	for ( const TPair<TSubclassOf<AUnit>, int32>& pair : waveData )
	{
		totalEnemies += pair.Value;
	}
	if ( IsValid( Text_TotalEnemies ) )
	{
		Text_TotalEnemies->SetText( FText::AsNumber( totalEnemies ) );
	}

	bool bNeedsRebuild = ( waveData.Num() != ActiveRowsMap_.Num() );
	if ( !bNeedsRebuild )
	{
		for ( const TPair<TSubclassOf<AUnit>, int32>& pair : waveData )
		{
			if ( !ActiveRowsMap_.Contains( pair.Key ) )
			{
				bNeedsRebuild = true;
				break;
			}
		}
	}

	if ( bNeedsRebuild )
	{
		EnemyListContainer->ClearChildren();
		ActiveRowsMap_.Empty();

		TArray<TPair<TSubclassOf<AUnit>, int32>> entries;
		entries.Reserve( waveData.Num() );
		for ( const TPair<TSubclassOf<AUnit>, int32>& pair : waveData )
		{
			if ( IsValid( EnemyDataAsset ) && EnemyDataAsset->EnemyDataMap.Contains( pair.Key ) )
			{
				entries.Add( pair );
			}
		}

		const int32 columns = FMath::Max( 1, ColumnCount_ );
		const int32 total = entries.Num();

		UHorizontalBox* currentRow = nullptr;
		for ( int32 i = 0; i < total; ++i )
		{
			const int32 columnInRow = i % columns;
			const int32 rowIndex = i / columns;
			const int32 itemsInThisRow = FMath::Min( columns, total - rowIndex * columns );
			const bool bRowFull = ( itemsInThisRow >= columns );

			if ( columnInRow == 0 )
			{
				currentRow = NewObject<UHorizontalBox>( this );
				UVerticalBoxSlot* vbSlot = EnemyListContainer->AddChildToVerticalBox( currentRow );
				if ( vbSlot )
				{
					vbSlot->SetHorizontalAlignment( bRowFull ? HAlign_Fill : HAlign_Center );
					vbSlot->SetPadding( FMargin( 0.0f, 4.0f ) );
				}
			}

			const FEnemyUIData& uiData = EnemyDataAsset->EnemyDataMap[entries[i].Key];

			UEnemyRowWidget* rowWidget = CreateWidget<UEnemyRowWidget>( this, EnemyRowClass );
			if ( !IsValid( rowWidget ) || !IsValid( currentRow ) )
			{
				continue;
			}

			rowWidget->SetupRow( uiData, entries[i].Value );
			UHorizontalBoxSlot* hbSlot = currentRow->AddChildToHorizontalBox( rowWidget );
			if ( hbSlot )
			{
				if ( bRowFull )
				{
					hbSlot->SetSize( FSlateChildSize( ESlateSizeRule::Fill ) );
				}
				hbSlot->SetPadding( FMargin( 4.0f, 0.0f ) );
				hbSlot->SetHorizontalAlignment( HAlign_Center );
				hbSlot->SetVerticalAlignment( VAlign_Center );
			}

			ActiveRowsMap_.Add( entries[i].Key, rowWidget );
		}
	}
	else
	{
		for ( const TPair<TSubclassOf<AUnit>, int32>& pair : waveData )
		{
			if ( UEnemyRowWidget* row = ActiveRowsMap_[pair.Key] )
			{
				row->UpdateCount( pair.Value );
			}
		}
	}

	RefreshSlideDistance();
	ApplyButtonSize();
}

void UWaveInfoPanelWidget::OnButtonToggleClicked()
{
	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_NEXTWAVEINFO_CLICKED } );
	TogglePanel();
}

void UWaveInfoPanelWidget::OnButtonToggleHovered()
{
	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_NEXTWAVEINFO_HOVERED } );
	bButtonHovered_ = true;
}

void UWaveInfoPanelWidget::OnButtonToggleUnhovered()
{
	bButtonHovered_ = false;
}

void UWaveInfoPanelWidget::OpenPanel()
{
	if ( bIsOpen_ )
	{
		return;
	}

	bIsOpen_ = true;
	OnPanelStateChanged.Broadcast( true );
}

void UWaveInfoPanelWidget::ClosePanel()
{
	if ( !bIsOpen_ )
	{
		return;
	}

	bIsOpen_ = false;
	OnPanelStateChanged.Broadcast( false );
}

void UWaveInfoPanelWidget::TogglePanel()
{
	if ( bIsOpen_ )
	{
		ClosePanel();
	}
	else
	{
		OpenPanel();
	}
}
