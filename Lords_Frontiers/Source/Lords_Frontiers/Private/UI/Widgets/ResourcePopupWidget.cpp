// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/ResourcePopupWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"


namespace ResourcePopupColors
{
	static const FSlateColor cPositiveBase = FSlateColor( FLinearColor( 0.1f, 0.8f, 0.1f, 1.0f ) );
	static const FSlateColor cPositiveBonus = FSlateColor( FLinearColor( 1.0f, 0.85f, 0.0f, 1.0f ) );
	static const FSlateColor cNegative = FSlateColor( FLinearColor( 0.9f, 0.15f, 0.15f, 1.0f ) );
} 

void UResourcePopupWidget::SetData(UTexture2D* icon, int32 amount, bool bIsBonus, bool bIsRuined)
{
	if ( ResourceIcon && icon )
	{
		ResourceIcon->SetBrushFromTexture( icon );
	}

	if ( AmountText )
	{
		const FString prefix = amount >= 0 ? TEXT( "+" ) : TEXT( "" );
		AmountText->SetText( FText::FromString( FString::Printf( TEXT( "%s%d" ), *prefix, amount ) ) );

		if ( amount < 0 || bIsRuined )
		{
			AmountText->SetColorAndOpacity( ResourcePopupColors::cNegative );
		}
		else if ( bIsBonus )
		{
			AmountText->SetColorAndOpacity( ResourcePopupColors::cPositiveBonus );
		}
		else
		{
			AmountText->SetColorAndOpacity( ResourcePopupColors::cPositiveBase );
		}
	}
}

void UResourcePopupWidget::SetPopupOpacity( float alpha )
{
	SetRenderOpacity( alpha );
}

void UResourcePopupWidget::SetIconOnly( bool bIconOnly )
{
	if ( AmountText )
	{
		AmountText->SetVisibility( bIconOnly ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible );
	}
}