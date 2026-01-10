// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/GameModes/MainMenuGameMode.h"

#include "UI/MainMenuUIManager.h"

void AMainMenuGameMode::InitGame( const FString& mapName, const FString& options, FString& errorMessage )
{
	Super::InitGame( mapName, options, errorMessage );

	UIManager_ = NewObject<UMainMenuUIManager>( this );
}

void AMainMenuGameMode::StartPlay()
{
	Super::StartPlay();

	UIManager_->SetupMainMenuWidget( MainMenuWidgetClass_ );
	UIManager_->OnStartPlay();
}
