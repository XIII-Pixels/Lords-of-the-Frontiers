// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UIManagers/UIManager.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

void UUIManager::OnStartPlay()
{
	if ( Widget_ )
	{
		Widget_->AddToViewport();

		if ( APlayerController* controller = Widget_->GetOwningPlayer() )
		{
			FInputModeUIOnly inputMode;
			inputMode.SetWidgetToFocus( Widget_->TakeWidget() );
			inputMode.SetLockMouseToViewportBehavior( EMouseLockMode::DoNotLock );
			controller->SetInputMode( inputMode );
			controller->SetShowMouseCursor( true );
		}
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT( "UUIManager: failed to show widget. Widget_ is nullptr" ) );
	}
}

void UUIManager::SetupWidget( TSubclassOf<UUserWidget> widgetClass )
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

	Widget_ = CreateWidget<UUserWidget>( controller, widgetClass );
}
