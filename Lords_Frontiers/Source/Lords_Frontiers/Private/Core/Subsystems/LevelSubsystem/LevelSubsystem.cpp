// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Subsystems/LevelSubsystem/LevelSubsystem.h"

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
		LoadLevel( Levels_->GameplayLevels[index], FString::Printf( TEXT( "gameplay level %d" ), index ) );
		CurrentLevelIndex_ = index;
	}
}

void ULevelSubsystem::LoadNextLevel()
{
	if ( Levels_ && CurrentLevelIndex_ >= -1 && CurrentLevelIndex_ + 1 < Levels_->GameplayLevels.Num() )
	{
		++CurrentLevelIndex_;
		LoadLevel(
		    Levels_->GameplayLevels[CurrentLevelIndex_], FString::Printf( TEXT( "level %d" ), CurrentLevelIndex_ )
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
	}
}

void ULevelSubsystem::LoadLevel( TSoftObjectPtr<UWorld> level, const FString& levelName ) const
{
	if ( !level.IsNull() )
	{
		UGameplayStatics::OpenLevel( GetWorld(), FName( *level.ToSoftObjectPath().GetLongPackageName() ) );
	}
	else
	{
		UE_LOG( LogTemp, Error, TEXT( "Failed to load %s" ), *levelName );
	}
}
