// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/LevelButton.h"

#include "Components/Button.h"
#include "Components/Image.h"

void ULevelButton::NativeConstruct()
{
	Super::NativeConstruct();

	if ( Butt )
	{
		Butt->OnClicked.AddDynamic( this, &ULevelButton::HandleClick );
	}
}

void ULevelButton::HandleClick()
{
	OnClicked.Broadcast( LevelIndex_ );
}

void ULevelButton::SetStateLocked()
{
	if ( Butt )
	{
		Butt->SetVisibility( ESlateVisibility::HitTestInvisible );
	}

	if ( StatusImage )
	{
		StatusImage->SetBrushFromTexture( TextureLocked_ );
		StatusImage->SetOpacity( 1.0f );
	}
	else
	{
		StatusImage->SetOpacity( 0.0f );
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
		StatusImage->SetBrushFromTexture( TextureUnlocked_ );
		StatusImage->SetOpacity( 1.0f );
	}
	else
	{
		StatusImage->SetOpacity( 0.0f );
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
		StatusImage->SetBrushFromTexture( TextureCompleted_ );
		StatusImage->SetOpacity( 1.0f );
	}
	else
	{
		StatusImage->SetOpacity( 0.0f );
	}
}
