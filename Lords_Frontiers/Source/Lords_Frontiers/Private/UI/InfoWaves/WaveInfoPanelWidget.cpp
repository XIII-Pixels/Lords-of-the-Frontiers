#include "Lords_Frontiers/Public/UI/InfoWaves/WaveInfoPanelWidget.h"

#include "Lords_Frontiers/Public/UI/InfoWaves/EnemyInfoDataAsset.h"
#include "Lords_Frontiers/Public/UI/InfoWaves/EnemyRowWidget.h"

#include "Components/HorizontalBox.h"

void UWaveInfoPanelWidget::PopulatePanel( const TMap<TSubclassOf<AUnit>, int32>& WaveData )
{
	if ( !EnemyListContainer || !EnemyRowClass )
		return;

	EnemyListContainer->ClearChildren();

	for ( const auto& Pair : WaveData )
	{
		TSubclassOf<AUnit> EnemyClass = Pair.Key;
		int32 Count = Pair.Value;

		if ( EnemyDataAsset && EnemyDataAsset->EnemyDataMap.Contains( EnemyClass ) )
		{
			FEnemyUIData UIData = EnemyDataAsset->EnemyDataMap[EnemyClass];

			UEnemyRowWidget* RowWidget = CreateWidget<UEnemyRowWidget>( this, EnemyRowClass );
			if ( RowWidget )
			{
				RowWidget->SetupRow( UIData, Count );
				EnemyListContainer->AddChildToHorizontalBox( RowWidget );
			}
		}
		else
		{
			UE_LOG(
			    LogTemp, Warning, TEXT( "Enemy data not found in DataAsset for class: %s" ), *EnemyClass->GetName()
			);
		}
	}
}