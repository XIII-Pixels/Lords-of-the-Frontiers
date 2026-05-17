// Fill out your copyright notice in the Description page of Project Settings.

#include "Sound/Data/MusicDataAsset.h"

const FMusicForLevel* UMusicDataAsset::MusicForLevel( TSoftObjectPtr<UWorld> level ) const
{
	if ( !level )
	{
		return nullptr;
	}

	const FString levelName = level.GetAssetName();

	for ( const auto& [key, value] : MusicForLevels_ )
	{
		if ( key.GetAssetName() == levelName )
		{
			return &value;
		}
	}

	return nullptr;
}
