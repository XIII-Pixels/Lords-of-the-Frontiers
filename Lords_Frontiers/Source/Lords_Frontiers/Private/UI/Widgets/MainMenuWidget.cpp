// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/MainMenuWidget.h"

#include "Core/Subsystems/LevelSubsystem/LevelSubsystem.h"

#include "Components/Button.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( NewGameButton )
	{
		NewGameButton->OnClicked.AddDynamic( this, &UMainMenuWidget::OnNewGameButtonClicked );
	}
}

void UMainMenuWidget::OnNewGameButtonClicked()
{
	GEngine->AddOnScreenDebugMessage( -1, 1.0f, FColor::Yellow, TEXT( "Button New Game Clicked" ) );
	GetGameInstance()->GetSubsystem<ULevelSubsystem>()->LoadRunLevel();
}
