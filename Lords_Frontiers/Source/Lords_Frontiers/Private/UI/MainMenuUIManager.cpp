// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/MainMenuUIManager.h"

#include "Blueprint/UserWidget.h"

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

void UMainMenuUIManager::SetupMainMenuWidget( TSubclassOf<UUserWidget> mainMenuWidgetClass )
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

	MainMenuWidget_ = CreateWidget<UUserWidget>( controller, mainMenuWidgetClass );
}
