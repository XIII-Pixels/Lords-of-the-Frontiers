// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Subsystems/LevelSubsystem/LevelSubsystem.h"

#include "Core/Saving/GameSaveData.h"
#include "Core/Saving/GameSaver.h"

#include "Kismet/GameplayStatics.h"

void ULevelSubsystem::LoadMainMenu() const
{
	if ( Levels_ )
	{
		LoadLevel( Levels_->MainMenuLevel, "main menu" );
	}
}

void ULevelSubsystem::LoadLevelChoosingLevel() const
{
	if ( Levels_ )
	{
		LoadLevel( Levels_->LevelChoosingLevel, "level choosing level" );
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

void ULevelSubsystem::SetupLevels( TSoftObjectPtr<ULevelsDataAsset> levels )
{
	if ( ULevelsDataAsset* loaded = levels.LoadSynchronous() )
	{
		Levels_ = loaded;
	}
	else
	{
		UE_LOG( LogTemp, Error, TEXT( "Failed to set up levels" ) );
		return;
	}

	if ( const auto* gameInstance = GetGameInstance() )
	{
		if ( const auto* gameSaver = gameInstance->GetSubsystem<UGameSaver>() )
		{
			for ( auto [level, unlocked] : Levels_->GameplayLevels )
			{
				if ( gameSaver->GetLevelStatus( level.ToSoftObjectPath().ToString() ) == ELevelStatus::Undefined )
				{
					gameSaver->UpdateLevelStatus(
					    level.ToSoftObjectPath().ToString(), unlocked ? ELevelStatus::Unlocked : ELevelStatus::Locked
					);
				}
			}
		}
	}
}

ELevelStatus ULevelSubsystem::GetLevelStatus( int index ) const
{
	if ( index >= 0 && index < Levels_->GameplayLevels.Num() )
	{
		if ( const auto* gameInstance = GetGameInstance() )
		{
			if ( const auto* gameSaver = gameInstance->GetSubsystem<UGameSaver>() )
			{
				return gameSaver->GetLevelStatus( Levels_->GameplayLevels[index].Level.ToSoftObjectPath().ToString() );
			}
		}
	}
	return ELevelStatus::Undefined;
}

void ULevelSubsystem::LoadLevel( TSoftObjectPtr<UWorld> level, const FString& levelName ) const
{
	if ( !level.IsNull() )
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr( GetWorld(), level );
	}
	else
	{
		UE_LOG( LogTemp, Error, TEXT( "Failed to load %s" ), *levelName );
	}
}
