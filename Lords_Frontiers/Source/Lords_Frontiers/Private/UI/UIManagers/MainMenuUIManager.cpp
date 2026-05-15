// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UIManagers/MainMenuUIManager.h"

#include "Core/Subsystems/LevelSubsystem/LevelSubsystem.h"
#include "UI/MainMenuWidget.h"

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Sound/AudioTags.h"
#include "sound/SoundEffectManager.h"

void UMainMenuUIManager::SetupWidget( TSubclassOf<UUserWidget> widgetClass )
{
	Super::SetupWidget( widgetClass );

	auto widget = Cast<UMainMenuWidget>( Widget_ );
	if ( widget && widget->NewGameButton )
	{
		widget->NewGameButton->OnClicked.AddDynamic( this, &UMainMenuUIManager::OnNewGameButtonClicked );
		widget->NewGameButton->OnHovered.AddDynamic( this, &UMainMenuUIManager::OnNewGameButtonHovered );
	}

	if ( widget && widget->ExitGameButton )
	{
		widget->ExitGameButton->OnClicked.AddDynamic( this, &UMainMenuUIManager::OnExitGameButtonClicked );
		widget->ExitGameButton->OnHovered.AddDynamic( this, &UMainMenuUIManager::OnExitGameButtonHovered );
	}
}

void UMainMenuUIManager::PostInitProperties()
{
	if ( const UWorld* world = GetWorld() )
	{
		if ( const UGameInstance* gameInstance = UGameplayStatics::GetGameInstance( world ) )
		{
			if ( USoundEffectManager* sfxManager = gameInstance->GetSubsystem<USoundEffectManager>() )
			{
				sfxManager->RegisterObject( this );
			}
		}
	}

	Super::PostInitProperties();
}

void UMainMenuUIManager::BeginDestroy()
{
	if ( const UWorld* world = GetWorld() )
	{
		if ( const UGameInstance* gameInstance = UGameplayStatics::GetGameInstance( world ) )
		{
			if ( USoundEffectManager* sfxManager = gameInstance->GetSubsystem<USoundEffectManager>() )
			{
				sfxManager->UnregisterObject( this );
			}
		}
	}

	Super::BeginDestroy();
}

void UMainMenuUIManager::OnNewGameButtonClicked()
{
	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_MAINMENU_BUTTONS_CLICKED } );
	UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<ULevelSubsystem>()->LoadLevelChoosingLevel();
}

void UMainMenuUIManager::OnNewGameButtonHovered()
{
	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_MAINMENU_BUTTONS_HOVERED } );
}

void UMainMenuUIManager::OnExitGameButtonClicked()
{
	OnAudioEvent_.Broadcast({ AudioTags::SFX_UI_MAINMENU_BUTTONS_CLICKED } );
	UKismetSystemLibrary::QuitGame( GetWorld(), nullptr, EQuitPreference::Quit, false );
}

void UMainMenuUIManager::OnExitGameButtonHovered()
{
	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_MAINMENU_BUTTONS_HOVERED } );
}
