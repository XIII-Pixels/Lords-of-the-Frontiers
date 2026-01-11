// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/MainMenuUIManager.h"

#include "Core/Subsystems/LevelSubsystem/LevelSubsystem.h"
#include "UI/Widgets/MainMenuWidget.h"

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UMainMenuUIManager::OnStartPlay()
{
	if ( MainMenuWidget_ )
	{
		MainMenuWidget_->AddToViewport();
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT( "Main Menu Widget is nullptr" ) );
	}
}

void UMainMenuUIManager::SetupMainMenuWidget( TSubclassOf<UMainMenuWidget> mainMenuWidgetClass )
{
	if ( !mainMenuWidgetClass )
	{
		UE_LOG( LogTemp, Warning, TEXT( "Main Menu Widget Class is not set" ) );
	}

	const auto* world = GetWorld();
	if ( !world )
	{
		UE_LOG( LogTemp, Error, TEXT( "Main Menu Widget setup: failed to get World" ) );
		return;
	}

	auto* controller = world->GetFirstPlayerController();
	if ( !controller )
	{
		UE_LOG( LogTemp, Error, TEXT( "Main Menu Widget setup: failed to get PlayerController" ) );
		return;
	}

	MainMenuWidget_ = CreateWidget<UMainMenuWidget>( controller, mainMenuWidgetClass );

	if ( MainMenuWidget_->NewGameButton )
	{
		MainMenuWidget_->NewGameButton->OnClicked.AddDynamic( this, &UMainMenuUIManager::OnNewGameButtonClicked );
	}
}

void UMainMenuUIManager::OnNewGameButtonClicked()
{
	UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<ULevelSubsystem>()->LoadRunLevel();
}
