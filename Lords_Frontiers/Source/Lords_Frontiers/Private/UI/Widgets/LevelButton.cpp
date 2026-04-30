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
