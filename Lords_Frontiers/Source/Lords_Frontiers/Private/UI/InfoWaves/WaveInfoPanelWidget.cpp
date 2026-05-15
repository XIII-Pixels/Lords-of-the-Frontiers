#include "UI/InfoWaves/WaveInfoPanelWidget.h"

#include "UI/InfoWaves/EnemyInfoDataAsset.h"
#include "UI/InfoWaves/EnemyRowWidget.h"
#include "Units/Unit.h"

#include "Components/Button.h"
#include "Components/PanelWidget.h"

void UWaveInfoPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( IsValid( BtnHandle ) )
	{
		BtnHandle->OnClicked.AddDynamic( this, &UWaveInfoPanelWidget::OnHandleClicked );
	}
}

void UWaveInfoPanelWidget::NativeTick( const FGeometry& myGeometry, float inDeltaTime )
{
	Super::NativeTick( myGeometry, inDeltaTime );

	float targetProgress = bIsOpen_ ? 1.0f : 0.0f;

	if ( !FMath::IsNearlyEqual( AnimProgress_, targetProgress ) )
	{
		float speed = ( AnimDuration_ > 0.0f ) ? ( 1.0f / AnimDuration_ ) : 10.0f;

		AnimProgress_ = FMath::FInterpConstantTo( AnimProgress_, targetProgress, inDeltaTime, speed );

		float curveValue = AnimProgress_;
		if ( IsValid( SlideCurve_ ) )
		{
			curveValue = SlideCurve_->GetFloatValue( AnimProgress_ );
		}

		if ( IsValid( SlideContainer ) && IsValid( PaperVisual ) )
		{
			SlideContainer->ForceLayoutPrepass();

			float paperHeight = PaperVisual->GetDesiredSize().Y;

			float targetY = -paperHeight + ClosedOffset_.Y;

			FVector2D dynamicClosedPos = FVector2D( 0.0f, targetY );
			FVector2D currentOffset = FMath::Lerp( dynamicClosedPos, OpenOffset_, curveValue );

			SlideContainer->SetRenderTranslation( currentOffset );
		}
	}
}

void UWaveInfoPanelWidget::PopulatePanel( const TMap<TSubclassOf<AUnit>, int32>& waveData )
{
	if ( !IsValid( EnemyListContainer ) || !IsValid( EnemyRowClass ) )
	{
		return;
	}

	bool bNeedsRebuild = false;

	if ( waveData.Num() != ActiveRowsMap_.Num() )
	{
		bNeedsRebuild = true;
	}
	else
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
		for ( const auto& kvp : ActiveRowsMap_ )
		{
			if ( IsValid( kvp.Value ) )
			{
				EnemyListContainer->RemoveChild( kvp.Value );
			}
		}
		ActiveRowsMap_.Empty();

		for ( const TPair<TSubclassOf<AUnit>, int32>& pair : waveData )
		{
			TSubclassOf<AUnit> enemyClass = pair.Key;

			if ( IsValid( EnemyDataAsset ) && EnemyDataAsset->EnemyDataMap.Contains( enemyClass ) )
			{
				FEnemyUIData uiData = EnemyDataAsset->EnemyDataMap[enemyClass];

				UEnemyRowWidget* rowWidget = CreateWidget<UEnemyRowWidget>( this, EnemyRowClass );
				if ( IsValid( rowWidget ) )
				{
					rowWidget->SetupRow( uiData, pair.Value );
					EnemyListContainer->AddChild( rowWidget );
					ActiveRowsMap_.Add( enemyClass, rowWidget );
				}
			}
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
}

void UWaveInfoPanelWidget::OnHandleClicked()
{
	TogglePanel();
}

void UWaveInfoPanelWidget::OpenPanel()
{
	bIsOpen_ = true;
}

void UWaveInfoPanelWidget::ClosePanel()
{
	bIsOpen_ = false;
}

void UWaveInfoPanelWidget::TogglePanel()
{
	bIsOpen_ = !bIsOpen_;
}