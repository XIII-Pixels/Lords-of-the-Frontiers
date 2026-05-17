#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "Sound/AudioSettingsSubsystem.h"

#include "AudioSettingsWidget.generated.h"

class UButton;
class USlider;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnAudioSettingsClosed );

/**
 * Volume settings widget with sliders for Music, Ambient, Effects and UI.
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
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnKeyDown( const FGeometry& InGeometry, const FKeyEvent& InKeyEvent ) override;
	virtual FReply NativeOnPreviewKeyDown( const FGeometry& InGeometry, const FKeyEvent& InKeyEvent ) override;

	void HandleClose();

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
	void UpdateValueText( UTextBlock* textBlock, float value ) const;
	void ApplySliderValue( EAudioCategory category, USlider* slider, UTextBlock* valueText, float value );
};
