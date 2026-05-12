#pragma once

#include "Core/GameLoop/GameLoopManager.h"

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "CoreMinimal.h"
#include "sound/AudioEvent.h"
#include "sound/AudioEventSource.h"

#include "CombatTimerWidget.generated.h"

class FOnAudioEvent;

UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UCombatTimerWidget : public UUserWidget, public IAudioEventSource
{
	GENERATED_BODY()

public:
	virtual FOnAudioEvent& GetOnAudioEvent() override
	{
		return OnAudioEvent_;
	}

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> TextTimer;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> ButtonPause;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> ButtonPlay;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> ButtonSpeedFast;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> ButtonSpeedTurbo;

	UPROPERTY(
	    EditAnywhere, BlueprintReadWrite, Category = "Settings|Speed", meta = ( ClampMin = "0.0", ClampMax = "32.0" )
	)
	float PauseSpeed = 0.0f;

	UPROPERTY(
	    EditAnywhere, BlueprintReadWrite, Category = "Settings|Speed", meta = ( ClampMin = "0.0", ClampMax = "32.0" )
	)
	float PlaySpeed = 1.0f;

	UPROPERTY(
	    EditAnywhere, BlueprintReadWrite, Category = "Settings|Speed", meta = ( ClampMin = "0.0", ClampMax = "32.0" )
	)
	float FastSpeed = 2.0f;

	UPROPERTY(
	    EditAnywhere, BlueprintReadWrite, Category = "Settings|Speed", meta = ( ClampMin = "0.0", ClampMax = "32.0" )
	)
	float TurboSpeed = 4.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Speed|Visuals" )
	float ActiveButtonOpacity = 1.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Speed|Visuals" )
	float InactiveButtonOpacity = 0.5f;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnPauseClicked();

	UFUNCTION()
	void OnPlayClicked();

	UFUNCTION()
	void OnSpeedFastClicked();

	UFUNCTION()
	void OnSpeedTurboClicked();

	UFUNCTION()
	void OnPauseHovered();

	UFUNCTION()
	void OnPlayHovered();

	UFUNCTION()
	void OnSpeedFastHovered();

	UFUNCTION()
	void OnSpeedTurboHovered();

	UFUNCTION()
	void HandleCombatTimer( float TimeRemaining, float TotalTime );

	UFUNCTION()
	void HandlePhaseChanged( EGameLoopPhase OldPhase, EGameLoopPhase NewPhase );

	UFUNCTION()
	void HandleSpeedChanged( float NewSpeed );

private:
	void ApplySpeed( float Speed );
	void UpdateActiveButtonVisuals( float Speed );
	void SetCombatVisible( bool bVisible );
	void UpdateTimerText( float TimeRemaining );

	FOnAudioEvent OnAudioEvent_;
};
