#include "UI/AudioSettingsWidget.h"

#include "Localization/GameLocalization.h"
#include "UI/Widgets/LanguageSwitchWidget.h"
#include "UI/Widgets/TextButtonWidget.h"

#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Input/Events.h"

namespace
{
	void SetLocalizedText( UTextBlock* textBlock, const TCHAR* key )
	{
		if ( textBlock )
		{
			textBlock->SetText( FText::FromStringTable( LordsFrontiersLoc::GetTableId(), key ) );
		}
	}
} // namespace

void UAudioSettingsWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	ApplyLocalizedLabels();
}

void UAudioSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable( true );

	ApplyLocalizedLabels();

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

	if ( OkButton )
	{
		OkButton->OnClicked.AddDynamic( this, &UAudioSettingsWidget::OnBackClicked );
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

void UAudioSettingsWidget::ApplyLocalizedLabels()
{
	SetLocalizedText( TitleText,         TEXT( "Settings.Title" ) );
	SetLocalizedText( MusicLabelText,    TEXT( "Settings.Volume.Music" ) );
	SetLocalizedText( AmbientLabelText,  TEXT( "Settings.Volume.Ambient" ) );
	SetLocalizedText( EffectsLabelText,  TEXT( "Settings.Volume.Effects" ) );
	SetLocalizedText( UILabelText,       TEXT( "Settings.Volume.UI" ) );
	SetLocalizedText( LanguageLabelText, TEXT( "Settings.Language" ) );
}

void UAudioSettingsWidget::InitializeSlider( USlider* slider, EAudioCategory category )
{
	if ( !slider )
	{
		return;
	}

	slider->SetMinValue( 0.0f );
	slider->SetMaxValue( 1.0f );
	// Keyboard/gamepad arrows adjust by exactly one step; mouse dragging is snapped to the
	// same grid in ApplySliderValue (USlider has no UMG setter for MouseUsesStep).
	slider->SetStepSize( VolumeStepSize );

	if ( const UGameInstance* gi = GetGameInstance() )
	{
		if ( const UAudioSettingsSubsystem* settings = gi->GetSubsystem<UAudioSettingsSubsystem>() )
		{
			slider->SetValue( SnapVolume( settings->GetVolume( category ) ) );
		}
	}
}

float UAudioSettingsWidget::SnapVolume( float value ) const
{
	if ( VolumeStepSize > KINDA_SMALL_NUMBER )
	{
		value = FMath::GridSnap( value, VolumeStepSize );
	}
	return FMath::Clamp( value, 0.0f, 1.0f );
}

void UAudioSettingsWidget::UpdateValueText( UTextBlock* textBlock, float value ) const
{
	if ( !textBlock )
	{
		return;
	}

	const float percent = FMath::Clamp( value, 0.0f, 1.0f ) * 100.0f;
	if ( VolumeStepSize < 0.01f - KINDA_SMALL_NUMBER )
	{
		textBlock->SetText( FText::FromString( FString::Printf( TEXT( "%.1f%%" ), percent ) ) );
	}
	else
	{
		textBlock->SetText( FText::FromString( FString::Printf( TEXT( "%d%%" ), FMath::RoundToInt( percent ) ) ) );
	}
}

void UAudioSettingsWidget::ApplySliderValue(
    EAudioCategory category, USlider* slider, UTextBlock* valueText, float value
)
{
	const float snapped = SnapVolume( value );

	// Push the snapped value back so the thumb sticks to the step grid while dragging;
	// USlider::SetValue does not re-fire OnValueChanged.
	if ( slider && !FMath::IsNearlyEqual( slider->GetValue(), snapped ) )
	{
		slider->SetValue( snapped );
	}

	if ( UGameInstance* gi = GetGameInstance() )
	{
		if ( UAudioSettingsSubsystem* settings = gi->GetSubsystem<UAudioSettingsSubsystem>() )
		{
			settings->SetVolume( category, snapped );
		}
	}
	UpdateValueText( valueText, snapped );
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
		HandleEscape();
		return FReply::Handled();
	}
	return Super::NativeOnPreviewKeyDown( InGeometry, InKeyEvent );
}

FReply UAudioSettingsWidget::NativeOnKeyDown( const FGeometry& InGeometry, const FKeyEvent& InKeyEvent )
{
	if ( InKeyEvent.GetKey() == EKeys::Escape )
	{
		HandleEscape();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown( InGeometry, InKeyEvent );
}

FReply UAudioSettingsWidget::NativeOnMouseButtonDown( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent )
{
	// A click that reached the window background (not consumed by a control)
	// folds the language dropdown instead of leaving it dangling open.
	if ( LanguageSwitch && LanguageSwitch->IsListOpen() )
	{
		LanguageSwitch->CloseList();
	}
	return Super::NativeOnMouseButtonDown( InGeometry, InMouseEvent );
}

void UAudioSettingsWidget::HandleEscape()
{
	// Escape folds the open language dropdown first; the window closes on the next press.
	if ( LanguageSwitch && LanguageSwitch->IsListOpen() )
	{
		LanguageSwitch->CloseList();
		return;
	}
	HandleClose();
}
