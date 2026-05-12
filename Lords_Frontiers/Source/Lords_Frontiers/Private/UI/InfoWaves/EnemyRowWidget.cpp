#include "UI/InfoWaves/EnemyRowWidget.h"

#include "UI/InfoWaves/EnemyInfoDataAsset.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UEnemyRowWidget::SetupRow( const FEnemyUIData& data, int32 count )
{
	if ( Text_Count )
	{
		Text_Count->SetText( FText::FromString( FString::Printf( TEXT( "x%d" ), count ) ) );
	}

	if ( Image_Icon && data.EnemyIcon )
	{
		Image_Icon->SetBrushFromTexture( data.EnemyIcon );
	}
}