// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Subsystems/LevelSubsystem/LevelSubsystem.h"

#include "Kismet/GameplayStatics.h"

void ULevelSubsystem::LoadMainMenu() const
{
	if ( Levels_ )
	{
		LoadLevel( Levels_->MainMenuLevel, "Failed to load main menu" );
	}
}

void ULevelSubsystem::LoadRunLevel() const
{
	if ( Levels_ )
	{
		LoadLevel( Levels_->RunLevel, "Failed to load run level" );
	}
}

void ULevelSubsystem::SetupLevels( TSoftObjectPtr<ULevelsDataAsset> levels )
{
	if ( levels )
	{
		Levels_ = levels;
	}
	else
	{
		UE_LOG( LogTemp, Error, TEXT( "Failed to set up levels" ) );
	}
}

void ULevelSubsystem::LoadLevel( TSoftObjectPtr<UWorld> level, const FString& errorMessage ) const
{
	if ( level.LoadSynchronous() )
	{
		UGameplayStatics::OpenLevel( GetWorld(), FName( level->GetName() ) );
	}
	else
	{
		UE_LOG( LogTemp, Error, TEXT( "%s" ), *errorMessage );
	}
}
