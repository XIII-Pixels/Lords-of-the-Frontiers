// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UIManagers/LevelChoosingUIManager.h"

#include "Core/Subsystems/LevelSubsystem/LevelSubsystem.h"
#include "UI/LevelChoosingMenu.h"
#include "UI/Widgets/LevelButton.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Widget.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/AudioTags.h"
#include "sound/SoundEffectManager.h"

void ULevelChoosingUIManager::SetupWidget( TSubclassOf<UUserWidget> widgetClass )
{
	Super::SetupWidget( widgetClass );

	const auto* menuWidget = Cast<ULevelChoosingMenu>( Widget_ );
	if ( !menuWidget )
	{
		return;
	}

	ULevelSubsystem* levelSubsystem = nullptr;
	if ( const UGameInstance* gameInstance = UGameplayStatics::GetGameInstance( GetWorld() ) )
	{
		levelSubsystem = gameInstance->GetSubsystem<ULevelSubsystem>();
	}

	TArray<UWidget*> allWidgets;
	if ( Widget_ )
	{
		Widget_->WidgetTree->GetAllWidgets( allWidgets );
	}

	for ( UWidget* widget : allWidgets )
	{
		if ( ULevelButton* levelButton = Cast<ULevelButton>( widget ) )
		{
			if ( levelButton->Butt != menuWidget->BackButton )
			{
				levelButton->OnClicked.AddDynamic( this, &ULevelChoosingUIManager::OnLevelButtonClicked );
				levelButton->Butt->OnHovered.AddDynamic( this, &ULevelChoosingUIManager::OnLevelButtonHovered );

				if ( levelSubsystem )
				{
					switch ( levelSubsystem->GetLevelStatus( levelButton->LevelIndex() ) )
					{
					case ELevelStatus::Unlocked:
						levelButton->SetStateUnlocked();
						break;
					case ELevelStatus::Completed:
						levelButton->SetStateCompleted();
						break;
					default:
						levelButton->SetStateLocked();
						break;
					}
				}
			}
		}
	}

	menuWidget->BackButton->OnClicked.AddDynamic( this, &ULevelChoosingUIManager::OnBackButtonClicked );
	menuWidget->BackButton->OnHovered.AddDynamic( this, &ULevelChoosingUIManager::OnBackButtonHovered );
}

void ULevelChoosingUIManager::PostInitProperties()
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

void ULevelChoosingUIManager::BeginDestroy()
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

void ULevelChoosingUIManager::OnLevelButtonClicked( int levelIndex )
{
	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_LEVELCHOOSINGMENU_BUTTONS_CLICKED } );
	if ( const UGameInstance* gameInstance = UGameplayStatics::GetGameInstance( GetWorld() ) )
	{
		if ( ULevelSubsystem* levelSubsystem = gameInstance->GetSubsystem<ULevelSubsystem>() )
		{
			levelSubsystem->LoadGameplayLevel( levelIndex );
		}
	}
}

void ULevelChoosingUIManager::OnLevelButtonHovered()
{
	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_LEVELCHOOSINGMENU_BUTTONS_HOVERED } );
}

void ULevelChoosingUIManager::OnBackButtonClicked()
{
	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_LEVELCHOOSINGMENU_BUTTONS_CLICKED } );
	if ( const UGameInstance* gameInstance = UGameplayStatics::GetGameInstance( GetWorld() ) )
	{
		if ( ULevelSubsystem* levelSubsystem = gameInstance->GetSubsystem<ULevelSubsystem>() )
		{
			levelSubsystem->LoadMainMenu();
		}
	}
}

void ULevelChoosingUIManager::OnBackButtonHovered()
{
	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_LEVELCHOOSINGMENU_BUTTONS_HOVERED } );
}
