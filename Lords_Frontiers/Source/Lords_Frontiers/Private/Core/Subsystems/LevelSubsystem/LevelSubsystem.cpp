// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Subsystems/LevelSubsystem/LevelSubsystem.h"

#include "Core/Saving/GameSaveData.h"
#include "Core/Saving/GameSaver.h"
#include "Core/Subsystems/TransitionSubsystem/TransitionSubsystem.h"

#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

void ULevelSubsystem::LoadMainMenu( bool bUseTransition ) const
{
	if ( Levels_ )
	{
		LoadLevel( Levels_->MainMenuLevel, "main menu", bUseTransition );
	}
}

void ULevelSubsystem::LoadLevelChoosingLevel() const
{
	if ( Levels_ )
	{
		// Menu -> level select is plain UI navigation: open it instantly, no transition wipe.
		LoadLevel( Levels_->LevelChoosingLevel, "level choosing level", /*bUseTransition*/ false );
	}
}

void ULevelSubsystem::LoadGameplayLevel( int index )
{
	if ( Levels_ && index >= 0 && index < Levels_->GameplayLevels.Num() )
	{
		LoadLevel( Levels_->GameplayLevels[index].Level, FString::Printf( TEXT( "gameplay level %d" ), index ) );
		CurrentLevelIndex_ = index;
	}
}

void ULevelSubsystem::LoadNextLevel()
{
	if ( Levels_ && CurrentLevelIndex_ >= -1 && CurrentLevelIndex_ + 1 < Levels_->GameplayLevels.Num() )
	{
		++CurrentLevelIndex_;
		LoadLevel(
		    Levels_->GameplayLevels[CurrentLevelIndex_].Level, FString::Printf( TEXT( "level %d" ), CurrentLevelIndex_ )
		);
	}
}

void ULevelSubsystem::SetLevels( TSoftObjectPtr<ULevelsDataAsset> levels )
{
	if ( ULevelsDataAsset* loaded = levels.LoadSynchronous() )
	{
		Levels_ = loaded;
	}
	else
	{
		UE_LOG( LogTemp, Error, TEXT( "ULevelSubsystem: Failed to load levels data asset" ) );
		return;
	}
}

void ULevelSubsystem::ResetSavedLevelStatuses() const
{
	if ( !Levels_ )
	{
		return;
	}
	if ( const auto* gameInstance = GetGameInstance() )
	{
		if ( const auto* gameSaver = gameInstance->GetSubsystem<UGameSaver>() )
		{
			for ( const FGameplayLevelData& levelData : Levels_->GameplayLevels )
			{
				if ( gameSaver->GetLevelStatus( levelData.Level.ToSoftObjectPath().ToString() ) ==
				     ELevelStatus::Undefined )
				{
					gameSaver->UpdateLevelStatus(
					    levelData.Level.GetAssetName(),
					    levelData.Unlocked ? ELevelStatus::Unlocked : ELevelStatus::Locked
					);
				}
			}
		}
	}
}

ELevelStatus ULevelSubsystem::GetLevelStatus( int index ) const
{
	if ( Levels_ && index >= 0 && index < Levels_->GameplayLevels.Num() )
	{
		if ( const auto* gameInstance = GetGameInstance() )
		{
			if ( const auto* gameSaver = gameInstance->GetSubsystem<UGameSaver>() )
			{
				return gameSaver->GetLevelStatus( Levels_->GameplayLevels[index].Level.GetAssetName() );
			}
		}
	}
	return ELevelStatus::Undefined;
}

bool ULevelSubsystem::GetCurrentLevelCameraConfig( FLevelCameraConfig& outConfig ) const
{
	if ( Levels_ && CurrentLevelIndex_ >= 0 && CurrentLevelIndex_ < Levels_->GameplayLevels.Num() )
	{
		outConfig = Levels_->GameplayLevels[CurrentLevelIndex_].CameraConfig;
		return true;
	}
	return false;
}

void ULevelSubsystem::UnlockNextLevel() const
{
	if ( !Levels_ )
	{
		return;
	}
	int index = CurrentLevelIndex_ + 1;
	if ( index >= 0 && index < Levels_->GameplayLevels.Num() )
	{
		if ( const auto* gameInstance = GetGameInstance() )
		{
			if ( const auto* gameSaver = gameInstance->GetSubsystem<UGameSaver>() )
			{
				const ELevelStatus currentStatus =
				    gameSaver->GetLevelStatus( Levels_->GameplayLevels[index].Level.GetAssetName() );
				if ( currentStatus == ELevelStatus::Locked || currentStatus == ELevelStatus::Undefined )
				{
					gameSaver->UpdateLevelStatus(
					    Levels_->GameplayLevels[index].Level.GetAssetName(), ELevelStatus::Unlocked
					);
				}
			}
		}
	}
}

void ULevelSubsystem::LoadLevel( TSoftObjectPtr<UWorld> level, const FString& levelName, bool bUseTransition ) const
{
	if ( level.IsNull() )
	{
		UE_LOG( LogTemp, Error, TEXT( "Failed to load %s" ), *levelName );
		return;
	}

	if ( bUseTransition )
	{
		if ( const UGameInstance* gameInstance = GetGameInstance() )
		{
			if ( UTransitionSubsystem* transition = gameInstance->GetSubsystem<UTransitionSubsystem>() )
			{
				transition->TransitionToSoftLevel( level );
				return;
			}
		}
	}

	UGameplayStatics::OpenLevelBySoftObjectPtr( GetWorld(), level );
}
