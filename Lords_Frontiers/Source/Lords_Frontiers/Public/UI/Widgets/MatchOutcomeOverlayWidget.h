#pragma once

#include "UI/Widgets/GameStateOverlayWidget.h"

#include "Core/GameSessionController.h"       // EGameResult
#include "UI/Widgets/MainMenuButtonWidget.h"  // UMainMenuButtonWidget, EMainMenuButtonAction

#include "MatchOutcomeOverlayWidget.generated.h"

class UTextBlock;

/**
 * Victory / Defeat overlay shown by UGameHUDWidget when a match ends. It derives from the
 * shared UGameStateOverlayWidget so it still slots into the existing ActiveOverlay / Win/Lose
 * WidgetClass plumbing, but the pause menu (a plain UGameStateOverlayWidget) is left untouched.
 *
 * Adds a localized result title (ResultTitleText) and two glow buttons (RestartButton /
 * MainMenuButton, WBP_MainMenuButton instances). Drive the title with SetResult(); GameHUD
 * calls it right after creating the overlay.
 *
 * In the Win/Lose WBP bind the new widgets by name (RestartButton, MainMenuButton,
 * ResultTitleText) and ignore the plain ButtonMainMenu / ButtonRestart inherited from the base.
 */
UCLASS( Abstract )
class LORDS_FRONTIERS_API UMatchOutcomeOverlayWidget : public UGameStateOverlayWidget
{
	GENERATED_BODY()

public:
	/** Sets the localized title: Win -> MatchResult.Win, Lose -> MatchResult.Lose,
	 *  EndlessRun -> MatchResult.Endless; anything else clears it. */
	UFUNCTION( BlueprintCallable, Category = "Results" )
	void SetResult( EGameResult result );

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void ApplyResultTitle();

	/** Localized Victory/Defeat title. Optional — name the Text Block "ResultTitleText" in the WBP. */
	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> ResultTitleText;

	// Glow buttons (WBP_MainMenuButton), named exactly RestartButton / MainMenuButton in the WBP.
	// C++ wires the clicks and assigns the MatchResult.Restart / MatchResult.MainMenu label key
	// when the instance has no LabelKey of its own.
	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UMainMenuButtonWidget> RestartButton;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UMainMenuButtonWidget> MainMenuButton;

	UFUNCTION()
	void HandleRestartClicked( EMainMenuButtonAction action );

	UFUNCTION()
	void HandleMainMenuClicked( EMainMenuButtonAction action );

	EGameResult CachedResult_ = EGameResult::Abandoned;
};
