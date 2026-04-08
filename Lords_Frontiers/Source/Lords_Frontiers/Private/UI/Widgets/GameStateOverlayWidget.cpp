#include "UI/Widgets/GameStateOverlayWidget.h"

#include "Core/CoreManager.h"
#include "Core/GameLoopManager.h"

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
	if ( TutorialButton )
		TutorialButton->OnClicked.AddDynamic( this, &UGameStateOverlayWidget::HandleTutorialButtonClicked );
}

void UGameStateOverlayWidget::OnMainMenuClicked()
{
	if ( UCoreManager* core = UCoreManager::Get( this ) )
		if ( UGameLoopManager* gL = core->GetGameLoop() )
			gL->Cleanup();

	UGameplayStatics::OpenLevel( this, mainMenuLevelName );
}

void UGameStateOverlayWidget::OnRestartClicked()
{
	if ( UCoreManager* core = UCoreManager::Get( this ) )
		if ( UGameLoopManager* gL = core->GetGameLoop() )
			gL->Cleanup();

	FString CurrentLevelName = GetWorld()->GetName();
	UGameplayStatics::OpenLevel( this, FName( *CurrentLevelName ) );
}

void UGameStateOverlayWidget::OnNextLevelClicked()
{
	if ( UCoreManager* core = UCoreManager::Get( this ) )
		if ( UGameLoopManager* gL = core->GetGameLoop() )
			gL->Cleanup();

	UGameplayStatics::OpenLevel( this, nextLevelName );
}

void UGameStateOverlayWidget::OnResumeClicked()
{
	OnResumeRequested.Broadcast();
}

void UGameStateOverlayWidget::HandleTutorialButtonClicked()
{
	if ( ADebugPlayerController* pc = Cast<ADebugPlayerController>( GetOwningPlayer() ) )
	{
		pc->ToggleTutorial();
	}
}