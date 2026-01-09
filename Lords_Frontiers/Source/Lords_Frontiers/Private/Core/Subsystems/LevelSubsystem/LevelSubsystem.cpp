// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Subsystems/LevelSubsystem/LevelSubsystem.h"

#include "Kismet/GameplayStatics.h"

void ULevelSubsystem::LoadMainMenu() const
{
	OpenLevel( Levels_->MainMenuLevel );
}

void ULevelSubsystem::LoadRunLevel() const
{
	OpenLevel( Levels_->RunLevel );
}

void ULevelSubsystem::SetupLevels( TSubclassOf<ULevelsDataAsset> levelsClass )
{
	Levels_ = NewObject<ULevelsDataAsset>( this, levelsClass );
}

void ULevelSubsystem::OpenLevel( TSoftObjectPtr<UWorld> level ) const
{
	UGameplayStatics::OpenLevel( GetWorld(), FName( level->GetName() ) );
}
