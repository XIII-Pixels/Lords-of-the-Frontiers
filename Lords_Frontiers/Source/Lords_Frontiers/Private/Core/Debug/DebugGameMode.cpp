// Fill out your copyright notice in the Description page of Project Settings.


#include "Lords_Frontiers/Public/Core/Debug/DebugGameMode.h"

#include "Blueprint/UserWidget.h"


void ADebugGameMode::PostInitProperties()
{
	Super::PostInitProperties();

	if ( DebugHUDClass )
	{
		DebugHUD_ = NewObject<UUserWidget>(this, DebugHUDClass);
	}

	if ( DebugHUD_ )
	{
		DebugHUD_->AddToViewport();
	}
}

void ADebugGameMode::BeginPlay()
{
	Super::BeginPlay();

	ShowMouseCursor();
}

/** (Gregory-hub)
 * By default game locks and hides mouse cursor. This method unlocks and shows it
 */
void ADebugGameMode::ShowMouseCursor() const
{
	if ( APlayerController* playerController = GetWorld()->GetFirstPlayerController() )
	{
		playerController->bShowMouseCursor = true;
		playerController->bEnableClickEvents = true;
		playerController->bEnableMouseOverEvents = true;
	}
}
