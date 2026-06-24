#pragma once

#include "Core/GameLoop/GameLoopManager.h"

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "Sound/AudioEvent.h"
#include "Sound/AudioEventSource.h"

#include "EndTurnButtonWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnEndTurnRequested );

/**
 * Standalone "End Turn" button (WBP_EndTurnButton), extracted from WBP_GameHUD.
 * Self-contained: localizes its label (HUD.EndTurn by default), calls
 * UGameLoopManager::EndBuildTurn() on click, plays its own hover/click sounds
 * and collapses itself outside the Building phase.
 * Optional GlowImage gets the idle/hovered tint lerp, so the UI glow materials
 * work here too (Doc/MainMenu_Glow_Setup.md, Doc/Glow_Flicker_Setup.md).
 */
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UEndTurnButtonWidget : public UUserWidget, public IAudioEventSource
{
	GENERATED_BODY()

public:
	UPROPERTY( BlueprintReadOnly, Category = "EndTurnButton", meta = ( BindWidget ) )
	TObjectPtr<UButton> RootButton;

	UPROPERTY( BlueprintReadOnly, Category = "EndTurnButton", meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> ButtonText;

	UPROPERTY( BlueprintReadOnly, Category = "EndTurnButton", meta = ( BindWidgetOptional ) )
	TObjectPtr<UImage> GlowImage;

	/** Key in ST_GameStrings. Takes priority over LabelText. */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "EndTurnButton", meta = ( ExposeOnSpawn = "true" ) )
	FName LabelKey = TEXT( "HUD.EndTurn" );

	/** Plain label used when LabelKey is None. */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "EndTurnButton", meta = ( ExposeOnSpawn = "true" ) )
	FText LabelText;

	/** Collapse the button outside the Building phase (the old WBP_GameHUD behavior). */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "EndTurnButton" )
	bool bAutoHideOutsideBuildPhase = true;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "EndTurnButton|Glow" )
	FLinearColor GlowColorIdle = FLinearColor( 1.0f, 0.85f, 0.4f, 0.25f );

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "EndTurnButton|Glow" )
	FLinearColor GlowColorHovered = FLinearColor( 1.0f, 0.9f, 0.55f, 1.0f );

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "EndTurnButton|Glow", meta = ( ClampMin = "0.0" ) )
	float GlowTransitionTime = 0.15f;

	/** Fires after the click has been forwarded to UGameLoopManager::EndBuildTurn(). */
	UPROPERTY( BlueprintAssignable, Category = "EndTurnButton" )
	FOnEndTurnRequested OnEndTurnRequested;

	UFUNCTION( BlueprintCallable, Category = "EndTurnButton" )
	void SetLabelKey( FName key );

	UFUNCTION( BlueprintCallable, Category = "EndTurnButton" )
	void SetLabelText( FText text );

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick( const FGeometry& myGeometry, float inDeltaTime ) override;

	// Hook for extra WBP polish on hover, mirrors MainMenuButtonWidget.
	UFUNCTION( BlueprintImplementableEvent, Category = "EndTurnButton" )
	void OnGlowProgressChanged( float progress );

	virtual FOnAudioEvent& GetOnAudioEvent() override
	{
		return OnAudioEvent_;
	}

private:
	UFUNCTION() void HandleClicked();
	UFUNCTION() void HandleHovered();
	UFUNCTION() void HandleUnhovered();
	UFUNCTION() void HandlePhaseChanged( EGameLoopPhase oldPhase, EGameLoopPhase newPhase );

	void ApplyLabel();
	void ApplyGlow();
	void UpdatePhaseVisibility( EGameLoopPhase phase );

	UGameLoopManager* GetGameLoop() const;

	FOnAudioEvent OnAudioEvent_;
	bool bIsHovered_ = false;
	float GlowProgress_ = 0.0f;
};
