// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "LevelsDataAsset.generated.h"

/** (Gregory-hub)
 * Per-level camera tuning: how far the camera can zoom out and where its zoom starts. */
USTRUCT()
struct FLevelCameraConfig
{
	GENERATED_BODY()

	/** When unchecked the camera keeps its own default zoom settings on this level. */
	UPROPERTY( EditDefaultsOnly, Category = "Camera", meta = ( DisplayName = "Переопределить камеру" ) )
	bool bOverrideCamera = false;

	/** Maximum camera height — how far the camera is allowed to zoom out (отдаление). */
	UPROPERTY(
	    EditDefaultsOnly, Category = "Camera",
	    meta = ( EditCondition = "bOverrideCamera", ClampMin = "1.0",
	             DisplayName = "Максимальная высота (отдаление)" )
	)
	float MaxHeight = 5000.0f;

	/** Starting camera height as a percent of the zoom range: 0% = fully zoomed in, 100% = fully zoomed out. */
	UPROPERTY(
	    EditDefaultsOnly, Category = "Camera",
	    meta = ( EditCondition = "bOverrideCamera", ClampMin = "0.0", ClampMax = "100.0", Units = "Percent",
	             DisplayName = "Стартовая высота (%)" )
	)
	float StartHeightPercent = 50.0f;
};

USTRUCT()
struct FGameplayLevelData
{
	GENERATED_BODY()

	UPROPERTY( EditDefaultsOnly )
	TSoftObjectPtr<UWorld> Level;

	UPROPERTY(
	    EditDefaultsOnly,
	    meta =
	        ( Tooltip = "Warning: changes are applied only if GameSaverConfig bClearAllSaveDataOnGameStart is checked" )
	)
	bool Unlocked = false;

	UPROPERTY( EditDefaultsOnly, meta = ( DisplayName = "Настройки камеры" ) )
	FLevelCameraConfig CameraConfig;
};

/** (Gregory-hub)
 * Contains all game levels */
UCLASS()
class LORDS_FRONTIERS_API ULevelsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditDefaultsOnly, Category = "Settings|Levels" )
	TSoftObjectPtr<UWorld> MainMenuLevel;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Levels" )
	TSoftObjectPtr<UWorld> LevelChoosingLevel;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Levels" )
	TArray<FGameplayLevelData> GameplayLevels;
};
