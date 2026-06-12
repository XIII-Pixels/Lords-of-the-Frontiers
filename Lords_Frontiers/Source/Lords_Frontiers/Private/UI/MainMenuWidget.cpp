// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/MainMenuWidget.h"

#include "UI/AudioSettingsWidget.h"

#include "Blueprint/WidgetTree.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	MenuButtons_.Reset();
	if ( WidgetTree )
	{
		WidgetTree->ForEachWidget(
		    [this]( UWidget* widget )
		    {
			    if ( auto* button = Cast<UMainMenuButtonWidget>( widget ) )
			    {
				    MenuButtons_.Add( button );
			    }
		    }
		);
	}

	for ( UMainMenuButtonWidget* button : MenuButtons_ )
	{
		button->OnClicked.AddDynamic( this, &UMainMenuWidget::HandleButtonClicked );
		button->OnHovered.AddDynamic( this, &UMainMenuWidget::HandleButtonHovered );
	}
}

void UMainMenuWidget::NativeDestruct()
{
	for ( UMainMenuButtonWidget* button : MenuButtons_ )
	{
		if ( button )
		{
			button->OnClicked.RemoveDynamic( this, &UMainMenuWidget::HandleButtonClicked );
			button->OnHovered.RemoveDynamic( this, &UMainMenuWidget::HandleButtonHovered );
		}
	}
	MenuButtons_.Reset();

	Super::NativeDestruct();
}

void UMainMenuWidget::HandleButtonClicked( EMainMenuButtonAction action )
{
	if ( action == EMainMenuButtonAction::Settings )
	{
		OpenAudioSettings();
	}

	OnActionRequested.Broadcast( action );
}

void UMainMenuWidget::HandleButtonHovered( EMainMenuButtonAction action )
{
	OnButtonHovered.Broadcast( action );
}

void UMainMenuWidget::OpenAudioSettings()
{
	if ( !AudioSettingsWidgetClass || ActiveAudioSettings_ )
	{
		return;
	}

	ActiveAudioSettings_ = CreateWidget<UAudioSettingsWidget>( this, AudioSettingsWidgetClass );
	if ( !ActiveAudioSettings_ )
	{
		return;
	}

	ActiveAudioSettings_->OnClosed.AddDynamic( this, &UMainMenuWidget::OnAudioSettingsClosed );
	ActiveAudioSettings_->AddToViewport( 100 );
}

void UMainMenuWidget::OnAudioSettingsClosed()
{
	ActiveAudioSettings_ = nullptr;
}
