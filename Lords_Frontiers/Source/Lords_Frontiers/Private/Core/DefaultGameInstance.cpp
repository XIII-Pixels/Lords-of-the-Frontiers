// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/DefaultGameInstance.h"

#include "Core/Subsystems/LevelSubsystem/LevelSubsystem.h"

void UDefaultGameInstance::Init()
{
	Super::Init();
	GetSubsystem<ULevelSubsystem>()->SetupLevels( Levels_ );
}
