#include "UI/Widgets/GameStateOverlayWidget.h"

#include "Core/CoreManager.h"
#include "Core/GameLoopManager.h"

#include "Camera/StrategyCamera.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UGameStateOverlayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( BtnMainMenu )
		BtnMainMenu->OnClicked.AddDynamic( this, &UGameStateOverlayWidget::OnMainMenuClicked );
	if ( BtnRestart )
		BtnRestart->OnClicked.AddDynamic( this, &UGameStateOverlayWidget::OnRestartClicked );
	if ( BtnNextLevel )
		BtnNextLevel->OnClicked.AddDynamic( this, &UGameStateOverlayWidget::OnNextLevelClicked );
	if ( BtnResume )
		BtnResume->OnClicked.AddDynamic( this, &UGameStateOverlayWidget::OnResumeClicked );
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
	RemoveFromParent();

	if ( APlayerController* PC = GetOwningPlayer() )
	{
		if ( AStrategyCamera* Cam = Cast<AStrategyCamera>( PC->GetPawn() ) )
		{
			Cam->SetCameraInputDisabled( false );
		}
	}
}