// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/DefaultGameInstance.h"

#include "Core/Saving/GameSaver.h"
#include "Core/Saving/GameSaverConfig.h"
#include "Core/Subsystems/LevelSubsystem/LevelSubsystem.h"

void UDefaultGameInstance::Init()
{
	Super::Init();

	const UGameSaver* gameSaver = GetSubsystem<UGameSaver>();
	ULevelSubsystem* levelSubsystem = GetSubsystem<ULevelSubsystem>();
	bool mustClearSaveData = GameSaverConfig && GameSaverConfig->bClearAllSaveDataOnGameStart;

#if !UE_BUILD_SHIPPING
	if ( mustClearSaveData )
	{
		if ( gameSaver )
		{
			gameSaver->Clear();
		}
		if ( levelSubsystem )
		{
			levelSubsystem->ResetSavedLevelStatuses();
		}
	}
#endif

	if ( levelSubsystem )
	{
		levelSubsystem->SetLevels( Levels );
	}

	if ( gameSaver && !gameSaver->HasLaunchedBefore() )
	{
		if ( levelSubsystem )
		{
			levelSubsystem->ResetSavedLevelStatuses();
		}
		gameSaver->MarkHasLaunched();
	}
}
