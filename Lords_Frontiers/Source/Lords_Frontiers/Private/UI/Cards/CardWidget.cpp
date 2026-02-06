#include "UI/Cards/CardWidget.h"

#include "Cards/CardDataAsset.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UCardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( CardButton )
	{
		CardButton->OnClicked.AddDynamic( this, &UCardWidget::HandleButtonClicked );
		CardButton->OnHovered.AddDynamic( this, &UCardWidget::HandleButtonHovered );
		CardButton->OnUnhovered.AddDynamic( this, &UCardWidget::HandleButtonUnhovered );
	}

	UpdateBorderVisual();

	if ( BackgroundImage && CardBackgroundTexture )
	{
		BackgroundImage->SetBrushFromTexture( CardBackgroundTexture );
	}
}

void UCardWidget::NativeDestruct()
{
	if ( CardButton )
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
	UpdateBorderVisual();

	OnSelectionChanged( bIsSelected_ );
}

void UCardWidget::SetInteractionEnabled( bool bEnabled )
{
	bIsInteractionEnabled_ = bEnabled;

	if ( CardButton )
	{
		CardButton->SetIsEnabled( bEnabled );
	}

	if ( BorderImage )
	{
		BorderImage->SetColorAndOpacity( bEnabled ? NormalTint : DisabledTint );
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

	if ( BorderImage && !bIsSelected_ )
	{
		BorderImage->SetColorAndOpacity( HoveredTint );
	}

	OnCardHovered();
}

void UCardWidget::HandleButtonUnhovered()
{
	if ( BorderImage && !bIsSelected_ )
	{
		BorderImage->SetColorAndOpacity( NormalTint );
	}

	OnCardUnhovered();
}

void UCardWidget::UpdateBorderVisual()
{
	if ( !BorderImage )
	{
		return;
	}

	UTexture2D* textureToUse = bIsSelected_ ? SelectedBorderTexture : NormalBorderTexture;

	if ( textureToUse )
	{
		BorderImage->SetBrushFromTexture( textureToUse );
	}

	BorderImage->SetColorAndOpacity( bIsInteractionEnabled_ ? NormalTint : DisabledTint );
}

void UCardWidget::UpdateCardVisuals()
{
	if ( !CardData_ )
	{
		return;
	}

	if ( CardNameText )
	{
		CardNameText->SetText( CardData_->CardName );
	}

	if ( DescriptionText )
	{
		DescriptionText->SetText( CardData_->GetFormattedDescription() );
	}

	if ( TargetText )
	{
		TargetText->SetText( CardData_->GetTargetDescription() );
	}

	if ( IconImage && CardData_->Icon )
	{
		IconImage->SetBrushFromTexture( CardData_->Icon );
		IconImage->SetVisibility( ESlateVisibility::Visible );
	}
	else if ( IconImage )
	{
		IconImage->SetVisibility( ESlateVisibility::Hidden );
	}
}
