#include "UI/InfoWaves/WaveInfoPanelWidget.h"

#include "UI/InfoWaves/EnemyInfoDataAsset.h"
#include "UI/InfoWaves/EnemyRowWidget.h"
#include "Units/Unit.h"

#include "Components/PanelWidget.h"

void UWaveInfoPanelWidget::NativeTick( const FGeometry& myGeometry, float inDeltaTime )
{
	Super::NativeTick( myGeometry, inDeltaTime );

	float targetProgress = bIsOpen_ ? 1.0f : 0.0f;

	if ( !FMath::IsNearlyEqual( AnimProgress_, targetProgress ) )
	{
		float speed = ( AnimDuration_ > 0.0f ) ? ( 1.0f / AnimDuration_ ) : 10.0f;

		if ( bIsOpen_ )
		{
			AnimProgress_ = FMath::Clamp( AnimProgress_ + ( inDeltaTime * speed ), 0.0f, 1.0f );
		}
		else
		{
			AnimProgress_ = FMath::Clamp( AnimProgress_ - ( inDeltaTime * speed ), 0.0f, 1.0f );
		}

		float curveValue = AnimProgress_;
		if ( IsValid( SlideCurve_ ) )
		{
			curveValue = SlideCurve_->GetFloatValue( AnimProgress_ );
		}

		FVector2D currentOffset = FMath::Lerp( ClosedOffset_, OpenOffset_, curveValue );

		if ( IsValid( SlideContainer ) )
		{
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
		EnemyListContainer->ClearChildren();
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