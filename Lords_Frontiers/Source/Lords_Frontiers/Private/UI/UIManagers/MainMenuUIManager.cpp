// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UIManagers/MainMenuUIManager.h"

#include "Core/Subsystems/LevelSubsystem/LevelSubsystem.h"
#include "UI/Widgets/MainMenuWidget.h"

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenuUIManager::SetupWidget( TSubclassOf<UUserWidget> widgetClass )
{
	const auto* world = GetWorld();
	if ( !world )
	{
		UE_LOG( LogTemp, Error, TEXT( "UMainMenuUIManager: failed to get World" ) );
		return;
	}

	auto* controller = world->GetFirstPlayerController();
	if ( !controller )
	{
		UE_LOG( LogTemp, Error, TEXT( "UMainMenuUIManager: failed to get PlayerController" ) );
		return;
	}

	Widget_ = CreateWidget<UMainMenuWidget>( controller, widgetClass );

	auto widget = Cast<UMainMenuWidget>( Widget_ );
	if ( widget && widget->NewGameButton )
	{
		widget->NewGameButton->OnClicked.AddDynamic( this, &UMainMenuUIManager::OnNewGameButtonClicked );
	}

	if ( widget && widget->ExitGameButton )
	{
		widget->ExitGameButton->OnClicked.AddDynamic( this, &UMainMenuUIManager::OnExitGameButtonClicked );
	}
}

void UMainMenuUIManager::OnNewGameButtonClicked()
{
	UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<ULevelSubsystem>()->LoadRunLevel();
}

void UMainMenuUIManager::OnExitGameButtonClicked()
{
	UKismetSystemLibrary::QuitGame( GetWorld(), nullptr, EQuitPreference::Quit, false );
}