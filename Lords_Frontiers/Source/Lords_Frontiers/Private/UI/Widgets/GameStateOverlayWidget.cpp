#include "UI/Widgets/GameStateOverlayWidget.h"

#include "Core/GameSessionController.h"

#include "Camera/StrategyCamera.h"
#include "Components/Button.h"
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
}

void UGameStateOverlayWidget::OnMainMenuClicked()
{
	if ( UGameSessionController* session = GetGameInstance()->GetSubsystem<UGameSessionController>() )
	{
		session->EndGame( EGameResult::Abandoned );
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
	UGameplayStatics::OpenLevel( this, FName( *CurrentLevelName ) );
}

void UGameStateOverlayWidget::OnNextLevelClicked()
{
	if ( UGameSessionController* session = GetGameInstance()->GetSubsystem<UGameSessionController>() )
	{
		session->EndGame( EGameResult::Abandoned );
	}

	UGameplayStatics::OpenLevel( this, nextLevelName );
}

void UGameStateOverlayWidget::OnResumeClicked()
{
	OnResumeRequested.Broadcast();
}