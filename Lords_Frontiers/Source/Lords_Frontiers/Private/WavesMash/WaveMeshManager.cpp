#include "WavesMesh/WaveMeshManager.h"

#include "Core/CoreManager.h"
#include "Core/GameLoop/GameLoopManager.h"

#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC( LogWaveMesh, Log, All );

AWavePortalManager::AWavePortalManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWavePortalManager::PrepareWave( const UWaveData* WaveData, int32 WaveIndex )
{
	ClearWave();
	CachedWaveIndex_ = WaveIndex;

	if ( !GetWorld() || !WaveData )
	{
		return;
	}

	TMap<FName, int32> totalPerSpawnPoint;

	for ( const TPair<TSubclassOf<AUnit>, FEnemySpawnSettings>& pair : WaveData->EnemySpawnMap )
	{
		const FEnemySpawnSettings& spawnSettings = pair.Value;

		for ( const FPortalSpawnEntry& portalEntry : spawnSettings.Portals )
		{
			if ( portalEntry.Count <= 0 || portalEntry.SpawnPointId.IsNone() )
			{
				continue;
			}

			totalPerSpawnPoint.FindOrAdd( portalEntry.SpawnPointId ) += portalEntry.Count;
		}
	}

	for ( const TPair<FName, int32>& pair : totalPerSpawnPoint )
	{
		AEnemyGroupSpawnPoint* spawnPoint = AEnemyGroupSpawnPoint::FindSpawnPointById( this, pair.Key );

		if ( !spawnPoint )
		{
			continue;
		}

		spawnPoint->ApplyPortalVisualConfig();

		FActiveSpawnPointPortalState& state = ActivePortals_.FindOrAdd( pair.Key );
		state.SpawnPoint = spawnPoint;
		state.TotalSpawns = pair.Value;
		state.RemainingSpawns = pair.Value;
		state.bVisible = false;

		if ( bShowPortalsInBuildPhase && spawnPoint->PortalVisualConfig.bShowInBuildPhase )
		{
			ShowPortal( pair.Key );
		}
	}
}

void AWavePortalManager::ShowPreparedPortals()
{
	for ( TPair<FName, FActiveSpawnPointPortalState>& pair : ActivePortals_ )
	{
		ShowPortal( pair.Key );
	}
}

void AWavePortalManager::NotifyEnemySpawnStarted( FName SpawnPointId )
{
	FActiveSpawnPointPortalState* state = ActivePortals_.Find( SpawnPointId );
	if ( !state )
	{
		return;
	}

	if ( !state->bVisible )
	{
		ShowPortal( SpawnPointId );
	}

	state->RemainingSpawns = FMath::Max( 0, state->RemainingSpawns - 1 );

	const AEnemyGroupSpawnPoint* spawnPoint = state->SpawnPoint.Get();
	if ( !spawnPoint )
	{
		return;
	}

	if ( state->RemainingSpawns == 0 &&
	     spawnPoint->PortalVisualConfig.HidePolicy == EPortalHidePolicy::HideAfterLastSpawn )
	{
		HidePortal( SpawnPointId );
	}
}

void AWavePortalManager::EndWave()
{
	if ( !bHidePortalsOnWaveEnd )
	{
		return;
	}

	for ( const TPair<FName, FActiveSpawnPointPortalState>& pair : ActivePortals_ )
	{
		HidePortal( pair.Key );
	}
}

void AWavePortalManager::ClearWave()
{
	for ( const TPair<FName, FActiveSpawnPointPortalState>& pair : ActivePortals_ )
	{
		HidePortal( pair.Key );
	}

	ActivePortals_.Empty();
	CachedWaveIndex_ = INDEX_NONE;
}

void AWavePortalManager::ShowPortal( FName SpawnPointId )
{
	FActiveSpawnPointPortalState* state = ActivePortals_.Find( SpawnPointId );
	if ( !state )
	{
		return;
	}

	AEnemyGroupSpawnPoint* spawnPoint = state->SpawnPoint.Get();
	if ( !spawnPoint )
	{
		return;
	}

	spawnPoint->SetPortalVisible( true, bDisableCollisionWhenHidden );
	state->bVisible = true;
}

void AWavePortalManager::HidePortal( FName SpawnPointId )
{
	FActiveSpawnPointPortalState* state = ActivePortals_.Find( SpawnPointId );
	if ( !state )
	{
		return;
	}

	AEnemyGroupSpawnPoint* spawnPoint = state->SpawnPoint.Get();
	if ( !spawnPoint )
	{
		return;
	}

	spawnPoint->SetPortalVisible( false, bDisableCollisionWhenHidden );
	state->bVisible = false;
}