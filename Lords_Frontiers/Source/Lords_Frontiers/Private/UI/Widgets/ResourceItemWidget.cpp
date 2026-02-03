#include "Lords_Frontiers/Public/UI/Widgets/ResourceItemWidget.h"

void UResourceItemWidget::SetupItem( UTexture2D* iconTexture, int32 initialValue, int32 maxValue )
{
	if ( ResourceIcon && iconTexture )
	{
		ResourceIcon->SetBrushFromTexture( iconTexture );
	}
	UpdateAmount( initialValue, maxValue );
}

void UResourceItemWidget::UpdateAmount( int32 newAmount, int32 maxAmount )
{
	if ( ResourceAmountText )
	{
		FString resourceString = FString::Printf( TEXT( "%d" ), FMath::Max( 0, newAmount ) );
		ResourceAmountText->SetText( FText::FromString( resourceString ) );
	}
}
