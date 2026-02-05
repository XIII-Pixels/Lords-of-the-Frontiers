#include "UI/Cards/CardWidget.h"

#include "Cards/CardDataAsset.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UCardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind button events
	if ( CardButton )
	{
		CardButton->OnClicked.AddDynamic( this, &UCardWidget::HandleButtonClicked );
		CardButton->OnHovered.AddDynamic( this, &UCardWidget::HandleButtonHovered );
		CardButton->OnUnhovered.AddDynamic( this, &UCardWidget::HandleButtonUnhovered );
	}

	// Set initial visuals
	UpdateBorderVisual();

	if ( BackgroundImage && CardBackgroundTexture )
	{
		BackgroundImage->SetBrushFromTexture( CardBackgroundTexture );
	}
}

void UCardWidget::NativeDestruct()
{
	// Unbind button events
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

	// Notify Blueprint
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

	// Notify Blueprint
	OnSelectionChanged( bIsSelected_ );
}

void UCardWidget::SetInteractionEnabled( bool bEnabled )
{
	bIsInteractionEnabled_ = bEnabled;

	if ( CardButton )
	{
		CardButton->SetIsEnabled( bEnabled );
	}

	// Update visual tint
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

	// Broadcast click event - parent widget handles selection logic
	OnCardClicked.Broadcast( this );
}

void UCardWidget::HandleButtonHovered()
{
	if ( !bIsInteractionEnabled_ )
	{
		return;
	}

	// Apply hover tint (subtle brighten)
	if ( BorderImage && !bIsSelected_ )
	{
		BorderImage->SetColorAndOpacity( HoveredTint );
	}

	OnCardHovered();
}

void UCardWidget::HandleButtonUnhovered()
{
	// Restore normal tint
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

	// Reset tint when changing selection state
	BorderImage->SetColorAndOpacity( bIsInteractionEnabled_ ? NormalTint : DisabledTint );
}

void UCardWidget::UpdateCardVisuals()
{
	if ( !CardData_ )
	{
		return;
	}

	// Set card name
	if ( CardNameText )
	{
		CardNameText->SetText( CardData_->CardName );
	}

	// Set description with formatted values
	if ( DescriptionText )
	{
		DescriptionText->SetText( CardData_->GetFormattedDescription() );
	}

	// Set target description
	if ( TargetText )
	{
		TargetText->SetText( CardData_->GetTargetDescription() );
	}

	// Set icon
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
