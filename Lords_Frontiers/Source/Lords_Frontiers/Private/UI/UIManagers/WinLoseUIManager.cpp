// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UIManagers/WinLoseUIManager.h"

#include "Core/Subsystems/LevelSubsystem/LevelSubsystem.h"
#include "UI/Widgets/WinLoseWidget.h"

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UWinLoseUIManager::SetupWidget( TSubclassOf<UUserWidget> widgetClass )
{
	const auto* world = GetWorld();
	if ( !world )
	{
		UE_LOG( LogTemp, Error, TEXT( "UWinLoseUIManager: failed to get World" ) );
		return;
	}

	auto* controller = world->GetFirstPlayerController();
	if ( !controller )
	{
		UE_LOG( LogTemp, Error, TEXT( "UWinLoseUIManager: failed to get PlayerController" ) );
		return;
	}

	Widget_ = CreateWidget<UUserWidget>( controller, widgetClass );

	if ( const auto widget = Cast<UWinLoseWidget>( Widget_ ) )
	{
		if ( widget->MainMenuButton )
		{
			widget->MainMenuButton->OnClicked.AddDynamic( this, &UWinLoseUIManager::OnMainMenuButtonClicked );
		}

		if ( widget->NewGameButton )
		{
			widget->NewGameButton->OnClicked.AddDynamic( this, &UWinLoseUIManager::OnNewGameButtonClicked );
		}

		if ( widget->QuitButton )
		{
			widget->QuitButton->OnClicked.AddDynamic( this, &UWinLoseUIManager::OnQuitButtonClicked );
		}
	}
}

void UWinLoseUIManager::OnMainMenuButtonClicked()
{
	UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<ULevelSubsystem>()->LoadMainMenu();
}

void UWinLoseUIManager::OnNewGameButtonClicked()
{
	UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<ULevelSubsystem>()->LoadRunLevel();
}

void UWinLoseUIManager::OnQuitButtonClicked()
{
	if ( const UWorld* world = GetWorld() )
	{
		APlayerController* pc = world->GetFirstPlayerController();

		UKismetSystemLibrary::QuitGame( world, pc, EQuitPreference::Quit, false );
	}
}
