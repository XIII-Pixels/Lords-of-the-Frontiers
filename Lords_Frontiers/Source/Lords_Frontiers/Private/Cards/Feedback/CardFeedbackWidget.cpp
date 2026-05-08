#include "Cards/Feedback/CardFeedbackWidget.h"

#include "Camera/CameraZoomUtils.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"

DEFINE_LOG_CATEGORY_STATIC( LogCardFeedbackWidget, Log, All );

void UCardFeedbackWidget::SetIcon( UTexture2D* icon )
{
	if ( !IconImage && WidgetTree )
	{
		IconImage = Cast<UImage>( WidgetTree->FindWidget( FName( TEXT( "IconImage" ) ) ) );

		if ( !IconImage )
		{
			TArray<UWidget*> allWidgets;
			WidgetTree->GetAllWidgets( allWidgets );
			for ( UWidget* w : allWidgets )
			{
				if ( UImage* img = Cast<UImage>( w ) )
				{
					IconImage = img;
					break;
				}
			}
		}
	}

	if ( IconImage && icon )
	{
		IconImage->SetBrushFromTexture( icon );
	}
	else if ( !IconImage )
	{
		UE_LOG(
		    LogCardFeedbackWidget, Warning,
		    TEXT( "CardFeedbackWidget: no UImage found in widget tree; add an Image to WBP (ideally named 'IconImage'). Hiding widget." ) );
		SetVisibility( ESlateVisibility::Collapsed );
		return;
	}

	OnShow();
}

void UCardFeedbackWidget::ApplyCameraScale( float zoomAlpha )
{
	const float scale = CameraZoomUtils::ScaleFromZoomAlpha( zoomAlpha, MinScale_, MaxScale_ );

	FWidgetTransform transform = GetRenderTransform();
	transform.Scale = FVector2D( scale, scale );
	SetRenderTransform( transform );
}
