// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/GameModes/MainMenuGameMode.h"

#include "Core/DefaultGameInstance.h"

#include "Sound/MusicAmbientManager.h"

void AMainMenuGameMode::StartPlay()
{
	Super::StartPlay();

	if ( const auto* gameInstance = Cast<UDefaultGameInstance>( GetGameInstance() ) )
	{
		if ( auto* musicManager = gameInstance->GetSubsystem<UMusicAmbientManager>() )
		{
			musicManager->PlayMainMenuMusic();
		}
	}
}
