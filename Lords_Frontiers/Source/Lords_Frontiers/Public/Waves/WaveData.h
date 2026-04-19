#pragma once

#include "Lords_Frontiers/Public/Waves/Wave.h"

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "WaveData.generated.h"

USTRUCT( BlueprintType )
struct FPortalSpawnEntry
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Spawn" )
	FName SpawnPointId = NAME_None;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = ( ClampMin = "0" ) )
	int32 Count = 0;
};

USTRUCT( BlueprintType )
struct FEnemySpawnSettings
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Spawn" )
	float StartDelay = 0.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Spawn" )
	float SpawnInterval = 1.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Spawn" )
	TArray<FPortalSpawnEntry> Portals;
};


UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UWaveData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Wave" )
	TMap<TSubclassOf<AUnit>, FEnemySpawnSettings> EnemySpawnMap;
};