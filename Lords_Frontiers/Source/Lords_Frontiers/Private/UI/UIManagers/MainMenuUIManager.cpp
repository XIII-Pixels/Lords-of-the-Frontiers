// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UIManagers/MainMenuUIManager.h"

#include "Core/Subsystems/LevelSubsystem/LevelSubsystem.h"
#include "UI/MainMenuWidget.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Sound/AudioTags.h"
#include "sound/SoundEffectManager.h"

void UMainMenuUIManager::SetupWidget( TSubclassOf<UUserWidget> widgetClass )
{
	Super::SetupWidget( widgetClass );

	auto widget = Cast<UMainMenuWidget>( Widget_ );
	if ( widget )
	{
		widget->OnActionRequested.AddDynamic( this, &UMainMenuUIManager::OnMenuActionRequested );
		widget->OnButtonHovered.AddDynamic( this, &UMainMenuUIManager::OnMenuButtonHovered );
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

void UMainMenuUIManager::OnMenuActionRequested( EMainMenuButtonAction action )
{
	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_MAINMENU_BUTTONS_CLICKED } );

	switch ( action )
	{
	case EMainMenuButtonAction::NewGame:
		UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<ULevelSubsystem>()->LoadLevelChoosingLevel();
		break;
	case EMainMenuButtonAction::ExitGame:
		UKismetSystemLibrary::QuitGame( GetWorld(), nullptr, EQuitPreference::Quit, false );
		break;
	default:
		break;
	}
}

void UMainMenuUIManager::OnMenuButtonHovered( EMainMenuButtonAction action )
{
	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_MAINMENU_BUTTONS_HOVERED } );
}
