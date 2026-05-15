#pragma once

#include "Core/GameLoop/GameLoopManager.h"
#include "Lords_Frontiers/Public/Waves/EnemyGroupSpawnPoint.h"
#include "Core/GameSessionController.h"
#include "Lords_Frontiers/Public/Waves/WaveData.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "WaveMeshManager.generated.h"

class UCoreManager;

USTRUCT()
struct FActiveSpawnPointPortalState
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AEnemyGroupSpawnPoint> SpawnPoint;

	int32 TotalSpawns = 0;
	int32 RemainingSpawns = 0;
	bool bVisible = false;
};

UCLASS( BlueprintType, Blueprintable, meta = ( DisplayName = "Wave Portal Manager" ) )
class LORDS_FRONTIERS_API AWavePortalManager : public AActor
{
	GENERATED_BODY()

public:
	AWavePortalManager();

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Portal" )
	bool bShowPortalsInBuildPhase = true;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Portal" )
	bool bHidePortalsOnWaveEnd = true;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Portal" )
	bool bDisableCollisionWhenHidden = true;

	UFUNCTION( BlueprintCallable, Category = "Portal" )
	void PrepareWave( const UWaveData* WaveData, int32 WaveIndex );

	UFUNCTION( BlueprintCallable, Category = "Portal" )
	void ShowPreparedPortals();

	UFUNCTION( BlueprintCallable, Category = "Portal" )
	void NotifyEnemySpawnStarted( FName SpawnPointId );

	UFUNCTION( BlueprintCallable, Category = "Portal" )
	void EndWave();

	UFUNCTION( BlueprintCallable, Category = "Portal" )
	void ClearWave();

private:
	void ShowPortal( FName SpawnPointId );
	void HidePortal( FName SpawnPointId );

	UPROPERTY()
	TMap<FName, FActiveSpawnPointPortalState> ActivePortals_;

	int32 CachedWaveIndex_ = INDEX_NONE;
};