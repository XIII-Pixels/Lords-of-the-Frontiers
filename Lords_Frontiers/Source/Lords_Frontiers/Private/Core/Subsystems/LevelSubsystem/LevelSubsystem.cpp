// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Subsystems/LevelSubsystem/LevelSubsystem.h"

#include "Kismet/GameplayStatics.h"

void ULevelSubsystem::LoadMainMenu() const
{
	if ( Levels_ )
	{
		if ( UWorld* level = Levels_->MainMenuLevel.LoadSynchronous() )
		{
			OpenLevel( level );
		}
		else
		{
			UE_LOG( LogTemp, Error, TEXT( "Failed to load main menu" ) );
		}
	}
}

void ULevelSubsystem::LoadRunLevel() const
{
	if ( Levels_ )
	{
		if ( UWorld* level = Levels_->RunLevel.LoadSynchronous() )
		{
			OpenLevel( level );
		}
		else
		{
			UE_LOG( LogTemp, Error, TEXT( "Failed to load run level" ) );
		}
	}
}

void ULevelSubsystem::SetupLevels( TSoftObjectPtr<ULevelsDataAsset> levels )
{
	Levels_ = levels;
}

void ULevelSubsystem::OpenLevel( UWorld* level ) const
{
	if ( level )
	{
		UGameplayStatics::OpenLevel( GetWorld(), FName( level->GetName() ) );
	}
}
