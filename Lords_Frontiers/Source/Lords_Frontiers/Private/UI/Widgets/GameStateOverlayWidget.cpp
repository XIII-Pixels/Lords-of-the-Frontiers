#include "UI/Widgets/GameStateOverlayWidget.h"

#include "Core/GameSessionController.h"
#include "Core/Subsystems/TransitionSubsystem/TransitionSubsystem.h"
#include "UI/AudioSettingsWidget.h"

#include "Camera/StrategyCamera.h"
#include "Components/Button.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

void UGameStateOverlayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( ButtonMainMenu )
		ButtonMainMenu->OnClicked.AddDynamic( this, &UGameStateOverlayWidget::OnMainMenuClicked );
	if ( ButtonRestart )
		ButtonRestart->OnClicked.AddDynamic( this, &UGameStateOverlayWidget::OnRestartClicked );
	if ( ButtonNextLevel )
		ButtonNextLevel->OnClicked.AddDynamic( this, &UGameStateOverlayWidget::OnNextLevelClicked );
	if ( ButtonResume )
		ButtonResume->OnClicked.AddDynamic( this, &UGameStateOverlayWidget::OnResumeClicked );
	if ( ButtonSettings )
		ButtonSettings->OnClicked.AddDynamic( this, &UGameStateOverlayWidget::OnSettingsClicked );
}

void UGameStateOverlayWidget::OnMainMenuClicked()
{
	if ( UGameSessionController* session = GetGameInstance()->GetSubsystem<UGameSessionController>() )
	{
		session->EndGame( EGameResult::Abandoned );
	}

	if ( UTransitionSubsystem* transition = GetGameInstance()->GetSubsystem<UTransitionSubsystem>() )
	{
		transition->TransitionToLevelByName( mainMenuLevelName );
		return;
	}
	UGameplayStatics::OpenLevel( this, mainMenuLevelName );
}

void UGameStateOverlayWidget::OnRestartClicked()
{
	if ( UGameSessionController* session = GetGameInstance()->GetSubsystem<UGameSessionController>() )
	{
		session->EndGame( EGameResult::Abandoned );
	}
	FString CurrentLevelName = GetWorld()->GetName();
	if ( UTransitionSubsystem* transition = GetGameInstance()->GetSubsystem<UTransitionSubsystem>() )
	{
		transition->TransitionToLevelByName( FName( *CurrentLevelName ) );
		return;
	}
	UGameplayStatics::OpenLevel( this, FName( *CurrentLevelName ) );
}

void UGameStateOverlayWidget::OnNextLevelClicked()
{
	if ( UGameSessionController* session = GetGameInstance()->GetSubsystem<UGameSessionController>() )
	{
		session->EndGame( EGameResult::Abandoned );
	}

	if ( UTransitionSubsystem* transition = GetGameInstance()->GetSubsystem<UTransitionSubsystem>() )
	{
		transition->TransitionToLevelByName( nextLevelName );
		return;
	}
	UGameplayStatics::OpenLevel( this, nextLevelName );
}

void UGameStateOverlayWidget::OnResumeClicked()
{
	OnResumeRequested.Broadcast();
}

void UGameStateOverlayWidget::OnSettingsClicked()
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

	ActiveAudioSettings_->OnClosed.AddDynamic( this, &UGameStateOverlayWidget::OnAudioSettingsClosed );
	constexpr int32 SettingsZOrder = 3000;
	ActiveAudioSettings_->AddToViewport( SettingsZOrder );
}

void UGameStateOverlayWidget::OnAudioSettingsClosed()
{
	ActiveAudioSettings_ = nullptr;
}