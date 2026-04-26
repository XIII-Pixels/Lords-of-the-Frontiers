// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Saving/GameSaver.h"

#include "Core/Saving/GameSaveData.h"

#include "Kismet/GameplayStatics.h"

void UGameSaver::UpdateCurrentLevelStatus( ELevelStatus levelStatus ) const
{
	UpdateLevelStatus( UGameplayStatics::GetCurrentLevelName( this, true ), levelStatus );
}

void UGameSaver::UpdateLevelStatus( const FString& levelName, ELevelStatus levelStatus ) const
{
	if ( levelName.IsEmpty() )
	{
		return;
	}

	UGameSaveData* saveData = Cast<UGameSaveData>( UGameplayStatics::LoadGameFromSlot( SaveSlotName_, 0 ) );
	if ( !saveData )
	{
		saveData = Cast<UGameSaveData>( UGameplayStatics::CreateSaveGameObject( UGameSaveData::StaticClass() ) );
	}

	saveData->StatusLevels.FindOrAdd( levelName ) = levelStatus;
	UGameplayStatics::SaveGameToSlot( saveData, SaveSlotName_, 0 );
	UE_LOG(
	    LogTemp, Log, TEXT( "UGameSaver: status of %s was updated to %i" ), *levelName,
	    saveData->StatusLevels[levelName]
	);
}

ELevelStatus UGameSaver::GetCurrentLevelStatus() const
{
	return GetLevelStatus( UGameplayStatics::GetCurrentLevelName( this, true ) );
}

ELevelStatus UGameSaver::GetLevelStatus( const FString& levelName ) const
{
	if ( levelName.IsEmpty() )
	{
		return ELevelStatus::Undefined;
	}

	UGameSaveData* saveData = Cast<UGameSaveData>( UGameplayStatics::LoadGameFromSlot( SaveSlotName_, 0 ) );
	if ( saveData && saveData->StatusLevels.Contains( levelName ) )
	{
		UE_LOG( LogTemp, Log, TEXT( "UGameSaver: status of %s: %i" ), *levelName, saveData->StatusLevels[levelName] );
		return saveData->StatusLevels[levelName];
	}
	UE_LOG( LogTemp, Log, TEXT( "UGameSaver: status of %s: %i" ), *levelName, ELevelStatus::Undefined );
	return ELevelStatus::Undefined;
}

void UGameSaver::Clear() const
{
	UGameplayStatics::DeleteGameInSlot( SaveSlotName_, 0 );
	UE_LOG( LogTemp, Warning, TEXT( "UGameSaver: all save data was cleared for slot %s" ), *SaveSlotName_ );
}
