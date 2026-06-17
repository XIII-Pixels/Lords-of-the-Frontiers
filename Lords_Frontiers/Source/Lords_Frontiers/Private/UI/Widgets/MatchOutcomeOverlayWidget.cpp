#include "UI/Widgets/MatchOutcomeOverlayWidget.h"

#include "Core/GameSessionController.h"
#include "Core/Subsystems/LevelSubsystem/LevelSubsystem.h"

#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Localization/GameLocalization.h"

void UMatchOutcomeOverlayWidget::NativeConstruct()
{
	// Base wires the inherited pause/navigation buttons; in the Win/Lose WBP those are unbound,
	// so it is a no-op there.
	Super::NativeConstruct();

	if ( RestartButton )
	{
		RestartButton->OnClicked.AddUniqueDynamic( this, &UMatchOutcomeOverlayWidget::HandleRestartClicked );
		if ( RestartButton->LabelKey.IsNone() )
		{
			RestartButton->SetLabelKey( TEXT( "MatchResult.Restart" ) );
		}
	}
	if ( MainMenuButton )
	{
		MainMenuButton->OnClicked.AddUniqueDynamic( this, &UMatchOutcomeOverlayWidget::HandleMainMenuClicked );
		if ( MainMenuButton->LabelKey.IsNone() )
		{
			MainMenuButton->SetLabelKey( TEXT( "MatchResult.MainMenu" ) );
		}
	}

	ApplyResultTitle();
}

void UMatchOutcomeOverlayWidget::NativeDestruct()
{
	if ( RestartButton )
		RestartButton->OnClicked.RemoveDynamic( this, &UMatchOutcomeOverlayWidget::HandleRestartClicked );
	if ( MainMenuButton )
		MainMenuButton->OnClicked.RemoveDynamic( this, &UMatchOutcomeOverlayWidget::HandleMainMenuClicked );

	Super::NativeDestruct();
}

void UMatchOutcomeOverlayWidget::SetResult( EGameResult result )
{
	CachedResult_ = result;
	ApplyResultTitle();
}

void UMatchOutcomeOverlayWidget::ApplyResultTitle()
{
	if ( !ResultTitleText )
	{
		return;
	}

	FText title;
	switch ( CachedResult_ )
	{
	case EGameResult::Win: title = LF_LOC( "MatchResult.Win" ); break;
	case EGameResult::Lose: title = LF_LOC( "MatchResult.Lose" ); break;
	case EGameResult::EndlessRun: title = LF_LOC( "MatchResult.Endless" ); break;
	default: title = FText::GetEmpty(); break;  // Abandoned: no result title.
	}
	ResultTitleText->SetText( title );
}

void UMatchOutcomeOverlayWidget::HandleRestartClicked( EMainMenuButtonAction action )
{
	if ( UGameInstance* gameInstance = GetGameInstance() )
	{
		if ( UGameSessionController* session = gameInstance->GetSubsystem<UGameSessionController>() )
		{
			session->RestartGame();
		}
	}
	RemoveFromParent();
}

void UMatchOutcomeOverlayWidget::HandleMainMenuClicked( EMainMenuButtonAction action )
{
	if ( UGameInstance* gameInstance = GetGameInstance() )
	{
		if ( const ULevelSubsystem* levels = gameInstance->GetSubsystem<ULevelSubsystem>() )
		{
			levels->LoadMainMenu();
		}
	}
	RemoveFromParent();
}
