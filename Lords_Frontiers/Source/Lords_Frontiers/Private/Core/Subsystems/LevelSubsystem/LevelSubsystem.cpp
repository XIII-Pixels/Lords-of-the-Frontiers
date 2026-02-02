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

void ULevelSubsystem::LoadRunLevel() const
{
	if ( Levels_ )
	{
		LoadLevel( Levels_->RunLevel, "run level" );
	}
}

void ULevelSubsystem::LoadWinLevel() const
{
	if ( Levels_ )
	{
		LoadLevel( Levels_->WinLevel, "win level" );
	}
}

void ULevelSubsystem::LoadLoseLevel() const
{
	if ( Levels_ )
	{
		LoadLevel( Levels_->LoseLevel, "lose level" );
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
