// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UIManagers/LevelChoosingUIManager.h"

#include "Core/Subsystems/LevelSubsystem/LevelSubsystem.h"
#include "UI/LevelChoosingMenu.h"
#include "UI/Widgets/LevelButton.h"
#include "UI/Widgets/TextButtonWidget.h"

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
		ULevelButton* levelButton = Cast<ULevelButton>( widget );

		// Skip non-level-buttons and level buttons whose inner UButton failed to bind (a
		// BindWidget can resolve to null). The back button is a UTextButtonWidget, so the
		// Cast<ULevelButton> above already filters it out.
		if ( !levelButton || !levelButton->Butt )
		{
			continue;
		}

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

	// BackButton is a BindWidget: if the Blueprint widget is missing it (or it failed to bind) the
	// pointer is invalid, so guard the bind instead of dereferencing it blindly. This is the line
	// the StartPlay access violation hit.
	if ( menuWidget->BackButton )
	{
		menuWidget->BackButton->OnClicked.AddDynamic( this, &ULevelChoosingUIManager::OnBackButtonClicked );
		menuWidget->BackButton->OnHovered.AddDynamic( this, &ULevelChoosingUIManager::OnBackButtonHovered );
	}
	else
	{
		UE_LOG( LogTemp, Warning,
		    TEXT( "ULevelChoosingUIManager::SetupWidget: BackButton is not bound on '%s'." ),
		    *menuWidget->GetName() );
	}
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
			// Level select -> main menu is plain UI navigation: open it instantly, no transition
			// wipe / loading animation (mirrors LoadLevelChoosingLevel for the reverse direction).
			levelSubsystem->LoadMainMenu( /*bUseTransition*/ false );
		}
	}
}

void ULevelChoosingUIManager::OnBackButtonHovered()
{
	OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_LEVELCHOOSINGMENU_BUTTONS_HOVERED } );
}
