// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/MainMenuWidget.h"

#include "UI/AudioSettingsWidget.h"

#include "Components/Button.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( SettingsButton )
	{
		SettingsButton->OnClicked.AddDynamic( this, &UMainMenuWidget::OnSettingsClicked );
	}
}

void UMainMenuWidget::OnSettingsClicked()
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
