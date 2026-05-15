#include "UI/InfoWaves/EnemyRowWidget.h"

#include "UI/InfoWaves/EnemyInfoDataAsset.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UEnemyRowWidget::SetupRow( const FEnemyUIData& data, int32 count )
{
	if ( IsValid( Image_Icon ) && IsValid( data.EnemyIcon ) )
	{
		Image_Icon->SetBrushFromTexture( data.EnemyIcon );
	}

	UpdateCount( count );
}

void UEnemyRowWidget::UpdateCount( int32 count )
{
	if ( currentCount_ == count )
	{
		return;
	}

	currentCount_ = count;

	if ( IsValid( Text_Count ) )
	{
		Text_Count->SetText( FText::FromString( FString::Printf( TEXT( "x%d" ), count ) ) );

		if ( count <= 0 )
		{
			Text_Count->SetColorAndOpacity( FSlateColor( FLinearColor( 0.4f, 0.4f, 0.4f, 1.0f ) ) );
			if ( IsValid( Image_Icon ) )
			{
				Image_Icon->SetColorAndOpacity( FLinearColor( 0.4f, 0.4f, 0.4f, 1.0f ) );
			}
		}
		else
		{
			Text_Count->SetColorAndOpacity( FSlateColor( FLinearColor::White ) );
			if ( IsValid( Image_Icon ) )
			{
				Image_Icon->SetColorAndOpacity( FLinearColor::White );
			}
		}
	}
}