// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Saving/GameSaver.h"

#include "Core/Saving/GameSaveData.h"

#include "Kismet/GameplayStatics.h"

void UGameSaver::UpdateCurrentLevelStatus( ELevelStatus levelStatus ) const
{
	const UWorld* world = GetWorld();
	if ( !world )
	{
		return;
	}
	UpdateLevelStatus( world->GetPathName(), levelStatus );
}

void UGameSaver::UpdateLevelStatus( const FString& levelPath, ELevelStatus levelStatus ) const
{
	UGameSaveData* saveData = Cast<UGameSaveData>( UGameplayStatics::LoadGameFromSlot( SaveSlotName_, 0 ) );
	if ( !saveData )
	{
		saveData = Cast<UGameSaveData>( UGameplayStatics::CreateSaveGameObject( UGameSaveData::StaticClass() ) );
	}

	saveData->StatusLevels.Add( levelPath, levelStatus );
	UGameplayStatics::SaveGameToSlot( saveData, SaveSlotName_, 0 );
}

ELevelStatus UGameSaver::GetCurrentLevelStatus() const
{
	const UWorld* world = GetWorld();
	if ( !world )
	{
		return ELevelStatus::Undefined;
	}
	return GetLevelStatus( world->GetPathName() );
}

ELevelStatus UGameSaver::GetLevelStatus( const FString& levelPath ) const
{
	UGameSaveData* saveData = Cast<UGameSaveData>( UGameplayStatics::LoadGameFromSlot( SaveSlotName_, 0 ) );
	if ( saveData && saveData->StatusLevels.Contains( levelPath ) )
	{
		return saveData->StatusLevels[levelPath];
	}
	return ELevelStatus::Undefined;
}
