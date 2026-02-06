#include "WavesMesh/WaveMeshManager.h"

#include "Core/CoreManager.h"
#include "Core/GameLoopManager.h"

#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC( LogWaveMesh, Log, All );

AWaveMeshManager::AWaveMeshManager()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>( TEXT( "Root" ) );
}

void AWaveMeshManager::BeginPlay()
{
	Super::BeginPlay();

	if ( bHideAllOnStart )
	{
		HideAllManagedActors();
	}

	BindToGameLoop();

	if ( !bIsBound_ )
	{
		UCoreManager* core = UCoreManager::Get( this );
		if ( core )
		{
			core->OnSystemsReady.AddDynamic( this, &AWaveMeshManager::HandleSystemsReady );
			UE_LOG( LogWaveMesh, Log, TEXT( "GameLoop not ready yet, waiting for OnSystemsReady..." ) );
		}
		else
		{
			UE_LOG( LogWaveMesh, Error, TEXT( "CoreManager not available!" ) );
		}
	}
}

void AWaveMeshManager::EndPlay( const EEndPlayReason::Type endPlayReason )
{
	UnbindFromGameLoop();

	UCoreManager* core = UCoreManager::Get( this );
	if ( core )
	{
		core->OnSystemsReady.RemoveDynamic( this, &AWaveMeshManager::HandleSystemsReady );
	}

	Super::EndPlay( endPlayReason );
}

void AWaveMeshManager::HandleSystemsReady()
{
	UE_LOG( LogWaveMesh, Log, TEXT( "OnSystemsReady received, binding now..." ) );

	BindToGameLoop();

	UCoreManager* core = UCoreManager::Get( this );
	if ( core )
	{
		core->OnSystemsReady.RemoveDynamic( this, &AWaveMeshManager::HandleSystemsReady );
	}
}

void AWaveMeshManager::BindToGameLoop()
{
	if ( bIsBound_ )
	{
		return;
	}

	UCoreManager* core = UCoreManager::Get( this );
	if ( !core )
	{
		UE_LOG( LogWaveMesh, Warning, TEXT( "BindToGameLoop: CoreManager not available" ) );
		return;
	}

	UGameLoopManager* gameLoop = core->GetGameLoop();
	if ( !gameLoop )
	{
		UE_LOG( LogWaveMesh, Warning, TEXT( "BindToGameLoop: GameLoopManager not available" ) );
		return;
	}

	GameLoopManager_ = gameLoop;

	gameLoop->OnPhaseChanged.AddDynamic( this, &AWaveMeshManager::HandlePhaseChanged );
	gameLoop->OnWaveChanged.AddDynamic( this, &AWaveMeshManager::HandleWaveChanged );

	bIsBound_ = true;

	CachedWave_ = gameLoop->GetCurrentWave();
	if ( CachedWave_ > 0 )
	{
		ForceRefreshVisibility();
	}

	UE_LOG( LogWaveMesh, Log, TEXT( "Bound to GameLoopManager. Entries: %d" ), WaveEntries.Num() );
}

void AWaveMeshManager::UnbindFromGameLoop()
{
	if ( !bIsBound_ )
	{
		return;
	}

	if ( UGameLoopManager* gameLoop = GameLoopManager_.Get() )
	{
		gameLoop->OnPhaseChanged.RemoveDynamic( this, &AWaveMeshManager::HandlePhaseChanged );
		gameLoop->OnWaveChanged.RemoveDynamic( this, &AWaveMeshManager::HandleWaveChanged );
	}

	GameLoopManager_.Reset();
	bIsBound_ = false;

	UE_LOG( LogWaveMesh, Log, TEXT( "Unbound from GameLoopManager" ) );
}

void AWaveMeshManager::HideAllManagedActors()
{
	for ( const FWaveMeshEntry& entry : WaveEntries )
	{
		for ( AActor* actor : entry.Actors )
		{
			if ( actor )
			{
				SetActorVisible( actor, false, entry.bDisableCollisionWhenHidden );
			}
		}
	}
}

void AWaveMeshManager::SetActorVisible( AActor* actor, bool bVisible, bool bDisableCollision )
{
	if ( !actor )
	{
		return;
	}

	const bool bCurrentlyHidden = actor->IsHidden();
	if ( bCurrentlyHidden == bVisible )
	{
		actor->SetActorHiddenInGame( !bVisible );

		if ( bDisableCollision )
		{
			actor->SetActorEnableCollision( bVisible );
		}
	}
}

void AWaveMeshManager::HandlePhaseChanged( EGameLoopPhase OldPhase, EGameLoopPhase NewPhase )
{
	UGameLoopManager* gameLoop = GameLoopManager_.Get();
	if ( !gameLoop )
	{
		return;
	}

	const int32 currentWave = gameLoop->GetCurrentWave();

	UE_LOG(
	    LogWaveMesh, Log, TEXT( "Phase changed: %d -> %d, Wave: %d" ), static_cast<int32>( OldPhase ),
	    static_cast<int32>( NewPhase ), currentWave
	);

	if ( NewPhase == EGameLoopPhase::Paused )
	{
		return;
	}

	EvaluateAllEntries( currentWave, NewPhase );
}

void AWaveMeshManager::HandleWaveChanged( int32 CurrentWave, int32 TotalWaves )
{
	CachedWave_ = CurrentWave;
	UE_LOG( LogWaveMesh, Log, TEXT( "Wave changed: %d / %d" ), CurrentWave, TotalWaves );
}

void AWaveMeshManager::ForceRefreshVisibility()
{
	UGameLoopManager* gameLoop = GameLoopManager_.Get();
	if ( !gameLoop )
	{
		return;
	}

	const int32 currentWave = gameLoop->GetCurrentWave();
	const EGameLoopPhase currentPhase = gameLoop->GetCurrentPhase();

	if ( currentPhase == EGameLoopPhase::Paused )
	{
		return;
	}

	EvaluateAllEntries( currentWave, currentPhase );
}

void AWaveMeshManager::EvaluateAllEntries( int32 currentWave, EGameLoopPhase currentPhase )
{
	TMap<AActor*, bool> ActorVisibility;
	TMap<AActor*, bool> ActorCollisionDisable;

	for ( const FWaveMeshEntry& entry : WaveEntries )
	{
		const bool bEntryWantsVisible = ShouldEntryBeVisible( entry, currentWave, currentPhase );

		for ( AActor* actor : entry.Actors )
		{
			if ( !actor )
			{
				continue;
			}

			bool* existing = ActorVisibility.Find( actor );
			if ( existing )
			{
				*existing = *existing || bEntryWantsVisible;
			}
			else
			{
				ActorVisibility.Add( actor, bEntryWantsVisible );
			}

			bool* existingCollision = ActorCollisionDisable.Find( actor );
			if ( existingCollision )
			{
				*existingCollision = *existingCollision && entry.bDisableCollisionWhenHidden;
			}
			else
			{
				ActorCollisionDisable.Add( actor, entry.bDisableCollisionWhenHidden );
			}
		}
	}

	for ( auto& pair : ActorVisibility )
	{
		AActor* actor = pair.Key;
		const bool bVisible = pair.Value;
		const bool bCurrentlyHidden = actor->IsHidden();

		if ( bCurrentlyHidden == bVisible )
		{
			actor->SetActorHiddenInGame( !bVisible );

			const bool* bDisableCollision = ActorCollisionDisable.Find( actor );
			if ( bDisableCollision && *bDisableCollision )
			{
				actor->SetActorEnableCollision( bVisible );
			}

			UE_LOG(
			    LogWaveMesh, Log, TEXT( "Actor '%s' -> %s" ), *actor->GetName(),
			    bVisible ? TEXT( "SHOWN" ) : TEXT( "HIDDEN" )
			);
		}
	}
}

bool AWaveMeshManager::ShouldEntryBeVisible(
    const FWaveMeshEntry& entry, int32 currentWave, EGameLoopPhase currentPhase
) const
{
	const bool bInWaveRange =
	    ( currentWave >= entry.ShowOnWave ) && ( entry.HideAfterWave <= 0 || currentWave <= entry.HideAfterWave );

	if ( !bInWaveRange )
	{
		return false;
	}

	switch ( currentPhase )
	{
	case EGameLoopPhase::Building:
	case EGameLoopPhase::Startup:
	{
		return entry.bShowDuringBuildPhase;
	}
	case EGameLoopPhase::Combat:
	{
		return true;
	}
	case EGameLoopPhase::Reward:
	{
		if ( entry.HideAfterWave > 0 && currentWave >= entry.HideAfterWave )
		{
			return false;
		}
		return !entry.bHideOnCombatEnd;
	}
	case EGameLoopPhase::Victory:
	case EGameLoopPhase::Defeat:
	case EGameLoopPhase::None:
	case EGameLoopPhase::Paused:
	{
		return false;
	}
	}

	return false;
}

void AWaveMeshManager::LogAllEntries() const
{
#if !UE_BUILD_SHIPPING
	UE_LOG( LogWaveMesh, Log, TEXT( "--- WaveMeshManager: %d entries ---" ), WaveEntries.Num() );

	for ( int32 i = 0; i < WaveEntries.Num(); ++i )
	{
		const FWaveMeshEntry& entry = WaveEntries[i];
		const FString hideStr =
		    entry.HideAfterWave > 0 ? FString::Printf( TEXT( "%d" ), entry.HideAfterWave ) : TEXT( "never" );

		UE_LOG(
		    LogWaveMesh, Log, TEXT( "  [%d] '%s': Show=%d, Hide=%s, Actors=%d, BuildPhase=%s, HideOnCombatEnd=%s" ), i,
		    *entry.Label, entry.ShowOnWave, *hideStr, entry.Actors.Num(),
		    entry.bShowDuringBuildPhase ? TEXT( "Y" ) : TEXT( "N" ), entry.bHideOnCombatEnd ? TEXT( "Y" ) : TEXT( "N" )
		);
	}

	UE_LOG( LogWaveMesh, Log, TEXT( "------------------------------------" ) );
#endif
}