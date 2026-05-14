// Fill out your copyright notice in the Description page of Project Settings.

#include "Sound/Data/AmbientDataAsset.h"

const FAmbientForLevel* UAmbientDataAsset::AmbientForLevel( TSoftObjectPtr<UWorld> level ) const
{
	if ( !level )
	{
		return nullptr;
	}

	const FString levelName = level.GetAssetName();

	for ( const auto& [key, value] : AmbientForLevels_ )
	{
		if ( key.GetAssetName() == levelName )
		{
			return &value;
		}
	}

	return nullptr;
}
