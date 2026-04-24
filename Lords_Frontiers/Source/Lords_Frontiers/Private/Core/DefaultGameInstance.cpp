// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/DefaultGameInstance.h"

#include "Core/Saving/GameSaver.h"
#include "Core/Saving/GameSaverConfig.h"
#include "Core/Subsystems/LevelSubsystem/LevelSubsystem.h"

void UDefaultGameInstance::Init()
{
	Super::Init();

#if !UE_BUILD_SHIPPING
	if ( GameSaverConfig && GameSaverConfig->bClearAllSaveDataOnGameStart )
	{
		GetSubsystem<UGameSaver>()->Clear();
	}
#endif

	GetSubsystem<ULevelSubsystem>()->SetupLevels( Levels_ );
}
