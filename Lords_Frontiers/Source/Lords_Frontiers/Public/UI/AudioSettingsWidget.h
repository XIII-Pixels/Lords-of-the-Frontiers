#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "Sound/AudioSettingsSubsystem.h"

#include "AudioSettingsWidget.generated.h"

class UButton;
class ULanguageSwitchWidget;
class USlider;
class UTextBlock;
class UTextButtonWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnAudioSettingsClosed );

/**
 * Settings window: volume sliders for Music, Ambient, Effects and UI, a language switch
 * and an OK button. All captions are localized through ST_GameStrings.
 * Used by both the main menu and the pause menu.
 */
UCLASS( Abstract )
class LORDS_FRONTIERS_API UAudioSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY( BlueprintAssignable, Category = "Events" )
	FOnAudioSettingsClosed OnClosed;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnKeyDown( const FGeometry& InGeometry, const FKeyEvent& InKeyEvent ) override;
	virtual FReply NativeOnPreviewKeyDown( const FGeometry& InGeometry, const FKeyEvent& InKeyEvent ) override;
	virtual FReply NativeOnMouseButtonDown( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;

	void HandleClose();
	void HandleEscape();

	/**
	 * Slider granularity as a fraction of the full range: 0.01 = whole percents.
	 * Mouse dragging snaps to this step, keyboard/gamepad arrows adjust by exactly one step,
	 * so the stored volume always matches the displayed percent.
	 */
	UPROPERTY( EditDefaultsOnly, Category = "Audio", meta = ( ClampMin = "0.001", ClampMax = "0.25" ) )
	float VolumeStepSize = 0.01f;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<USlider> MusicSlider;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<USlider> AmbientSlider;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<USlider> EffectsSlider;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<USlider> UISlider;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> MusicValueText;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> AmbientValueText;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> EffectsValueText;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> UIValueText;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> MusicLabelText;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> AmbientLabelText;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> EffectsLabelText;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> UILabelText;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> LanguageLabelText;

	/** Reusable OK button widget (WBP based on UTextButtonWidget). Closes the window. */
	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextButtonWidget> OkButton;

	/** Language toggle (WBP based on ULanguageSwitchWidget). Self-contained. */
	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<ULanguageSwitchWidget> LanguageSwitch;

	/** Legacy plain-UButton close button; kept for widgets that have not migrated to OkButton. */
	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> ButtonBack;

	UFUNCTION()
	void OnMusicChanged( float value );

	UFUNCTION()
	void OnAmbientChanged( float value );

	UFUNCTION()
	void OnEffectsChanged( float value );

	UFUNCTION()
	void OnUIChanged( float value );

	UFUNCTION()
	void OnBackClicked();

private:
	void InitializeSlider( USlider* slider, EAudioCategory category );
	void ApplyLocalizedLabels();
	float SnapVolume( float value ) const;
	void UpdateValueText( UTextBlock* textBlock, float value ) const;
	void ApplySliderValue( EAudioCategory category, USlider* slider, UTextBlock* valueText, float value );
};
