#include "Lords_Frontiers/Public/UI/InfoWaves/EnemyRowWidget.h"

#include "Lords_Frontiers/Public/UI/InfoWaves/EnemyInfoDataAsset.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UEnemyRowWidget::SetupRow( const FEnemyUIData& Data, int32 Count )
{
	if ( Text_Count )
	{
		Text_Count->SetText( FText::FromString( FString::Printf( TEXT( "x%d" ), Count ) ) );
	}

	if ( Image_Icon && Data.EnemyIcon )
	{
		Image_Icon->SetBrushFromTexture( Data.EnemyIcon );
	}
}