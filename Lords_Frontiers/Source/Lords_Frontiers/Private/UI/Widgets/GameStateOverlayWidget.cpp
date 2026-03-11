#include "UI/Widgets/GameStateOverlayWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

#include "Core/CoreManager.h"
#include "Core/GameLoopManager.h"
#include "Camera/StrategyCamera.h"

void UGameStateOverlayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( BtnMainMenu )
	{
		BtnMainMenu->OnClicked.AddDynamic( this, &UGameStateOverlayWidget::OnMainMenuClicked );
	}
	if ( BtnRestart )
	{
		BtnRestart->OnClicked.AddDynamic( this, &UGameStateOverlayWidget::OnRestartClicked );
	}
	if ( BtnNextLevel )
	{
		BtnNextLevel->OnClicked.AddDynamic( this, &UGameStateOverlayWidget::OnNextLevelClicked );
	}
	if ( BtnResume )
	{
		BtnResume->OnClicked.AddDynamic( this, &UGameStateOverlayWidget::OnResumeClicked );
	}
}

void UGameStateOverlayWidget::SetupWinState()
{
	if ( TextTitle )
	{
		TextTitle->SetText( FText::FromString( TEXT( "Win!" ) ) );
	}
	if ( BtnMainMenu )
	{
		BtnMainMenu->SetVisibility( ESlateVisibility::Visible );
	}
	if ( BtnRestart )
	{
		BtnRestart->SetVisibility( ESlateVisibility::Visible );
	}
	if ( BtnNextLevel )
	{
		BtnNextLevel->SetVisibility( ESlateVisibility::Visible );
	}
	if ( BtnResume )
	{
		BtnResume->SetVisibility( ESlateVisibility::Collapsed );
	}
}

void UGameStateOverlayWidget::SetupLoseState()
{
	if ( TextTitle )
	{
		TextTitle->SetText( FText::FromString( TEXT( "Lose" ) ) );
	}
	if ( BtnMainMenu )
	{
		BtnMainMenu->SetVisibility( ESlateVisibility::Visible );
	}
	if ( BtnRestart )
	{
		BtnRestart->SetVisibility( ESlateVisibility::Visible );
	}
	if ( BtnNextLevel )
	{
		BtnNextLevel->SetVisibility( ESlateVisibility::Collapsed );
	}
	if ( BtnResume )
	{
		BtnResume->SetVisibility( ESlateVisibility::Collapsed );
	}
}

void UGameStateOverlayWidget::SetupPauseState()
{
	if ( TextTitle )
	{
		TextTitle->SetText( FText::FromString( TEXT( "Pause" ) ) );
	}
	if ( BtnMainMenu )
	{
		BtnMainMenu->SetVisibility( ESlateVisibility::Visible );
	}
	if ( BtnRestart )
	{
		BtnRestart->SetVisibility( ESlateVisibility::Visible );
	}
	if ( BtnNextLevel )
	{
		BtnNextLevel->SetVisibility( ESlateVisibility::Collapsed );
	}
	if ( BtnResume )
	{
		BtnResume->SetVisibility( ESlateVisibility::Visible );
	}
}

void UGameStateOverlayWidget::OnMainMenuClicked()
{
	UGameplayStatics::OpenLevel( this, mainMenuLevelName );
}

void UGameStateOverlayWidget::OnRestartClicked()
{
	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( UGameLoopManager* gL = core->GetGameLoop() )
		{
			gL->Cleanup();
		}
	}

	FString CurrentLevelName = GetWorld()->GetName();
	UGameplayStatics::OpenLevel( this, FName( *CurrentLevelName ) );
}

void UGameStateOverlayWidget::OnNextLevelClicked()
{
	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( UGameLoopManager* gL = core->GetGameLoop() )
		{
			gL->Cleanup();
		}
	}

	UGameplayStatics::OpenLevel( this, nextLevelName );
}

void UGameStateOverlayWidget::OnResumeClicked()
{
	SetVisibility( ESlateVisibility::Collapsed );

	if ( APlayerController* PC = GetOwningPlayer() )
	{
		if ( AStrategyCamera* Cam = Cast<AStrategyCamera>( PC->GetPawn() ) )
		{
			Cam->SetCameraInputDisabled( false );
		}
	}
}