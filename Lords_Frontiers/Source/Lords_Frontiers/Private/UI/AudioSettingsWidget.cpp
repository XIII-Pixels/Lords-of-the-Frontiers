#include "UI/AudioSettingsWidget.h"

#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Input/Events.h"

void UAudioSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable( true );

	InitializeSlider( MusicSlider,   EAudioCategory::Music );
	InitializeSlider( AmbientSlider, EAudioCategory::Ambient );
	InitializeSlider( EffectsSlider, EAudioCategory::Effects );
	InitializeSlider( UISlider,      EAudioCategory::UI );

	if ( MusicSlider )
	{
		MusicSlider->OnValueChanged.AddDynamic( this, &UAudioSettingsWidget::OnMusicChanged );
		UpdateValueText( MusicValueText, MusicSlider->GetValue() );
	}
	if ( AmbientSlider )
	{
		AmbientSlider->OnValueChanged.AddDynamic( this, &UAudioSettingsWidget::OnAmbientChanged );
		UpdateValueText( AmbientValueText, AmbientSlider->GetValue() );
	}
	if ( EffectsSlider )
	{
		EffectsSlider->OnValueChanged.AddDynamic( this, &UAudioSettingsWidget::OnEffectsChanged );
		UpdateValueText( EffectsValueText, EffectsSlider->GetValue() );
	}
	if ( UISlider )
	{
		UISlider->OnValueChanged.AddDynamic( this, &UAudioSettingsWidget::OnUIChanged );
		UpdateValueText( UIValueText, UISlider->GetValue() );
	}

	if ( ButtonBack )
	{
		ButtonBack->OnClicked.AddDynamic( this, &UAudioSettingsWidget::OnBackClicked );
	}

	SetKeyboardFocus();
}

void UAudioSettingsWidget::NativeDestruct()
{
	if ( UGameInstance* gi = GetGameInstance() )
	{
		if ( UAudioSettingsSubsystem* settings = gi->GetSubsystem<UAudioSettingsSubsystem>() )
		{
			settings->SaveSettings();
		}
	}
	Super::NativeDestruct();
}

void UAudioSettingsWidget::InitializeSlider( USlider* slider, EAudioCategory category )
{
	if ( !slider )
	{
		return;
	}

	slider->SetMinValue( 0.0f );
	slider->SetMaxValue( 1.0f );

	if ( const UGameInstance* gi = GetGameInstance() )
	{
		if ( const UAudioSettingsSubsystem* settings = gi->GetSubsystem<UAudioSettingsSubsystem>() )
		{
			slider->SetValue( settings->GetVolume( category ) );
		}
	}
}

void UAudioSettingsWidget::UpdateValueText( UTextBlock* textBlock, float value ) const
{
	if ( !textBlock )
	{
		return;
	}
	const int32 percent = FMath::RoundToInt( FMath::Clamp( value, 0.0f, 1.0f ) * 100.0f );
	textBlock->SetText( FText::FromString( FString::Printf( TEXT( "%d%%" ), percent ) ) );
}

void UAudioSettingsWidget::ApplySliderValue(
    EAudioCategory category, USlider* /*slider*/, UTextBlock* valueText, float value
)
{
	if ( UGameInstance* gi = GetGameInstance() )
	{
		if ( UAudioSettingsSubsystem* settings = gi->GetSubsystem<UAudioSettingsSubsystem>() )
		{
			settings->SetVolume( category, value );
		}
	}
	UpdateValueText( valueText, value );
}

void UAudioSettingsWidget::OnMusicChanged( float value )
{
	ApplySliderValue( EAudioCategory::Music, MusicSlider, MusicValueText, value );
}

void UAudioSettingsWidget::OnAmbientChanged( float value )
{
	ApplySliderValue( EAudioCategory::Ambient, AmbientSlider, AmbientValueText, value );
}

void UAudioSettingsWidget::OnEffectsChanged( float value )
{
	ApplySliderValue( EAudioCategory::Effects, EffectsSlider, EffectsValueText, value );
}

void UAudioSettingsWidget::OnUIChanged( float value )
{
	ApplySliderValue( EAudioCategory::UI, UISlider, UIValueText, value );
}

void UAudioSettingsWidget::OnBackClicked()
{
	HandleClose();
}

void UAudioSettingsWidget::HandleClose()
{
	if ( UGameInstance* gi = GetGameInstance() )
	{
		if ( UAudioSettingsSubsystem* settings = gi->GetSubsystem<UAudioSettingsSubsystem>() )
		{
			settings->SaveSettings();
		}
	}
	OnClosed.Broadcast();
	RemoveFromParent();
}

FReply UAudioSettingsWidget::NativeOnPreviewKeyDown( const FGeometry& InGeometry, const FKeyEvent& InKeyEvent )
{
	if ( InKeyEvent.GetKey() == EKeys::Escape )
	{
		HandleClose();
		return FReply::Handled();
	}
	return Super::NativeOnPreviewKeyDown( InGeometry, InKeyEvent );
}

FReply UAudioSettingsWidget::NativeOnKeyDown( const FGeometry& InGeometry, const FKeyEvent& InKeyEvent )
{
	if ( InKeyEvent.GetKey() == EKeys::Escape )
	{
		HandleClose();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown( InGeometry, InKeyEvent );
}
