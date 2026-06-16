// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/LevelButton.h"

#include "Localization/GameLocalization.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/RetainerBox.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"

void ULevelButton::NativePreConstruct()
{
	Super::NativePreConstruct();

	ApplyLabel();
	ApplyGlow();
}

void ULevelButton::NativeConstruct()
{
	Super::NativeConstruct();

	if ( Butt )
	{
		Butt->OnClicked.AddDynamic( this, &ULevelButton::HandleClick );
		Butt->OnHovered.AddDynamic( this, &ULevelButton::HandleHovered );
		Butt->OnUnhovered.AddDynamic( this, &ULevelButton::HandleUnhovered );
	}

	bIsHovered_ = false;
	GlowProgress_ = 0.0f;
	ApplyLabel();
	ApplyGlow();
}

void ULevelButton::NativeDestruct()
{
	if ( Butt )
	{
		Butt->OnClicked.RemoveDynamic( this, &ULevelButton::HandleClick );
		Butt->OnHovered.RemoveDynamic( this, &ULevelButton::HandleHovered );
		Butt->OnUnhovered.RemoveDynamic( this, &ULevelButton::HandleUnhovered );
	}

	Super::NativeDestruct();
}

void ULevelButton::NativeTick( const FGeometry& myGeometry, float inDeltaTime )
{
	Super::NativeTick( myGeometry, inDeltaTime );

	const float target = bIsHovered_ ? 1.0f : 0.0f;
	if ( FMath::IsNearlyEqual( GlowProgress_, target ) )
	{
		return;
	}

	if ( GlowTransitionTime <= KINDA_SMALL_NUMBER )
	{
		GlowProgress_ = target;
	}
	else
	{
		GlowProgress_ = FMath::FInterpConstantTo( GlowProgress_, target, inDeltaTime, 1.0f / GlowTransitionTime );
	}

	ApplyGlow();
}

void ULevelButton::HandleClick()
{
	OnClicked.Broadcast( LevelIndex_ );
}

void ULevelButton::HandleHovered()
{
	bIsHovered_ = true;
}

void ULevelButton::HandleUnhovered()
{
	bIsHovered_ = false;
}

void ULevelButton::SetLabelKey( FName key )
{
	LabelKey = key;
	ApplyLabel();
}

void ULevelButton::SetLabelText( FText text )
{
	LabelText = text;
	ApplyLabel();
}

void ULevelButton::SetStateLocked()
{
	if ( Butt )
	{
		Butt->SetVisibility( ESlateVisibility::HitTestInvisible );
	}

	if ( StatusImage )
	{
		if ( TextureLocked_ )
		{
			StatusImage->SetBrushFromTexture( TextureLocked_ );
			StatusImage->SetVisibility( ESlateVisibility::Visible );
		}
		else
		{
			StatusImage->SetVisibility( ESlateVisibility::Collapsed );
		}
	}
}

void ULevelButton::SetStateUnlocked()
{
	if ( Butt )
	{
		Butt->SetVisibility( ESlateVisibility::Visible );
	}

	if ( StatusImage )
	{
		if ( TextureUnlocked_ )
		{
			StatusImage->SetBrushFromTexture( TextureUnlocked_ );
			StatusImage->SetVisibility( ESlateVisibility::Visible );
		}
		else
		{
			StatusImage->SetVisibility( ESlateVisibility::Collapsed );
		}
	}
}

void ULevelButton::SetStateCompleted()
{
	if ( Butt )
	{
		Butt->SetVisibility( ESlateVisibility::Visible );
	}

	if ( StatusImage )
	{
		if ( TextureCompleted_ )
		{
			StatusImage->SetBrushFromTexture( TextureCompleted_ );
			StatusImage->SetVisibility( ESlateVisibility::Visible );
		}
		else
		{
			StatusImage->SetVisibility( ESlateVisibility::Collapsed );
		}
	}
}

void ULevelButton::ApplyLabel()
{
	if ( !ButtonText )
	{
		return;
	}

	if ( !LabelKey.IsNone() )
	{
		ButtonText->SetText( FText::FromStringTable( LordsFrontiersLoc::GetTableId(), LabelKey.ToString() ) );
	}
	else if ( !LabelText.IsEmpty() )
	{
		ButtonText->SetText( LabelText );
	}
}

void ULevelButton::ApplyGlow()
{
	const FLinearColor glowColor = FMath::Lerp( GlowColorIdle, GlowColorHovered, GlowProgress_ );

	if ( GlowImage )
	{
		GlowImage->SetColorAndOpacity( glowColor );
	}

	if ( GlowRetainer )
	{
		if ( UMaterialInstanceDynamic* material = GlowRetainer->GetEffectMaterial() )
		{
			material->SetVectorParameterValue( GlowColorParameter, glowColor );
			material->SetScalarParameterValue( GlowProgressParameter, GlowProgress_ );
		}
	}

	if ( ButtonText )
	{
		ButtonText->SetColorAndOpacity( FMath::Lerp( TextColorIdle, TextColorHovered, GlowProgress_ ) );

		if ( bGlowTextOutline )
		{
			FSlateFontInfo font = ButtonText->GetFont();
			font.OutlineSettings.OutlineColor = glowColor;
			ButtonText->SetFont( font );
		}
	}

	OnGlowProgressChanged( GlowProgress_ );
}
