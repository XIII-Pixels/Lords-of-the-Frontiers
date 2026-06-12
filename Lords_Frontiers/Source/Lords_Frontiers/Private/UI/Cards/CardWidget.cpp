#include "UI/Cards/CardWidget.h"

#include "Cards/CardDataAsset.h"
#include "Cards/CardTypes.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/RichTextBlock.h"

void UCardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( CardButton )
	{
		CardButton->OnClicked.AddDynamic( this, &UCardWidget::HandleButtonClicked );
		CardButton->OnHovered.AddDynamic( this, &UCardWidget::HandleButtonHovered );
		CardButton->OnUnhovered.AddDynamic( this, &UCardWidget::HandleButtonUnhovered );
	}

	SetRenderTransformPivot( FVector2D( 0.5f, 0.5f ) );
	SetRenderScale( FVector2D( 1.0f, 1.0f ) );
}

void UCardWidget::NativeDestruct()
{
	if ( IsValid( CardButton ) )
	{
		CardButton->OnClicked.RemoveDynamic( this, &UCardWidget::HandleButtonClicked );
		CardButton->OnHovered.RemoveDynamic( this, &UCardWidget::HandleButtonHovered );
		CardButton->OnUnhovered.RemoveDynamic( this, &UCardWidget::HandleButtonUnhovered );
	}

	Super::NativeDestruct();
}

void UCardWidget::SetCardData( UCardDataAsset* cardData )
{
	CardData_ = cardData;

	if ( CardData_ )
	{
		UpdateCardVisuals();
	}

	OnCardDataSet();
}

void UCardWidget::SetSelected( bool bSelected )
{
	if ( bIsSelected_ == bSelected )
	{
		return;
	}

	bIsSelected_ = bSelected;

	const float scale = bIsSelected_ ? SelectedScale : 1.0f;
	SetRenderScale( FVector2D( scale, scale ) );

	OnSelectionChanged( bIsSelected_ );
}

void UCardWidget::SetInteractionEnabled( bool bEnabled )
{
	bIsInteractionEnabled_ = bEnabled;

	if ( CardButton )
	{
		CardButton->SetIsEnabled( bEnabled );
	}
}

void UCardWidget::HandleButtonClicked()
{
	if ( !bIsInteractionEnabled_ )
	{
		return;
	}

	OnCardClicked.Broadcast( this );
}

void UCardWidget::HandleButtonHovered()
{
	if ( !bIsInteractionEnabled_ )
	{
		return;
	}

	OnCardHovered();
}

void UCardWidget::HandleButtonUnhovered()
{
	OnCardUnhovered();
}

static void ApplyIconTexture( UImage* image, UTexture2D* texture )
{
	if ( !image )
	{
		return;
	}

	if ( texture )
	{
		image->SetBrushFromTexture( texture );
		image->SetVisibility( ESlateVisibility::HitTestInvisible );
	}
	else
	{
		image->SetVisibility( ESlateVisibility::Hidden );
	}
}

void UCardWidget::UpdateCardVisuals()
{
	if ( !CardData_ )
	{
		return;
	}

	if ( CardNameText )
	{
		CardNameText->SetText( CardData_->GetCardName() );
	}

	if ( DescriptionText )
	{
		DescriptionText->SetText( CardData_->BuildDescription() );
	}

	ApplyIconTexture( BuildingIconImage, CardData_->BuildingIcon );
	ApplyIconTexture( FeatureIconImage, CardData_->FeatureIcon );

	const bool bHasFire = CardData_->EffectFrames.Contains( ECardEffectFrame::Fire );
	const bool bHasSlow = CardData_->EffectFrames.Contains( ECardEffectFrame::Slow );

	UTexture2D* effectFrameTexture = nullptr;
	if ( bHasFire && bHasSlow )
	{
		effectFrameTexture = EffectFrameSlowFireTexture;
	}
	else if ( bHasFire )
	{
		effectFrameTexture = EffectFrameFireTexture;
	}
	else if ( bHasSlow )
	{
		effectFrameTexture = EffectFrameSlowTexture;
	}

	ApplyIconTexture( EffectFrameIconImage, effectFrameTexture );
}
