// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/GameModes/UIOnlyGameMode.h"

#include "UI/UIManagers/UIManager.h"

#include "Blueprint/UserWidget.h"

void AUIOnlyGameMode::InitGame( const FString& mapName, const FString& options, FString& errorMessage )
{
	Super::InitGame( mapName, options, errorMessage );

	UIManager_ = NewObject<UUIManager>( this, UIManagerClass_ );
}

void AUIOnlyGameMode::StartPlay()
{
	Super::StartPlay();

	UIManager_->SetupWidget( WidgetClass_ );
	UIManager_->OnStartPlay();
}
