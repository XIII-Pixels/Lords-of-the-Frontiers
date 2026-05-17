#include "Lords_Frontiers/Public/Waves/WaveManager.h"

#include "AI/Path/Path.h"
#include "Cards/Visuals/CardVisualSubsystem.h"
#include "Core/CoreManager.h"
#include "Core/GameLoop/GameLoopManager.h"
#include "Lords_Frontiers/Public/Waves/Infinite/InfiniteModeConfig.h"
#include "Lords_Frontiers/Public/Waves/Infinite/InfiniteWaveBuilder.h"
#include "Lords_Frontiers/Public/Waves/WaveData.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "WavesMesh/WaveMeshManager.h"
#if WITH_EDITOR
#include "Editor.h"
#endif

#include "Units/UnitBuilder.h"

#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "NavigationSystem.h"
#include "UObject/ScriptDelegates.h"

#define TIME_TO_END_WAVE_AFTER_LAST_DEATH 1.0f

AWaveManager::AWaveManager()
{
	PrimaryActorTick.bCanEverTick = false;
	CurrentWaveIndex = 0;
	bAutoStartOnBeginPlay = false;
	bLogSpawning = true;
	bIsWaveActive_ = false;
}

void AWaveManager::BeginPlay()
{
	Super::BeginPlay();

	if ( WaveConfig_ )
	{
		ApplyWaveConfig();
		BuildSelectedWavePresetCache();
	}

	EnsureInfiniteBuilder();

	if ( bAutoStartOnBeginPlay )
	{
		StartWaves();
	}
	BuildSelectedWavePresetCache();
}

bool AWaveManager::HasInfiniteMode() const
{
	return InfiniteConfig != nullptr;
}

bool AWaveManager::IsInfiniteWaveIndex( int32 waveIndex ) const
{
	if ( !InfiniteConfig )
	{
		return false;
	}
	return waveIndex >= InfiniteConfig->StartWaveIndex;
}

void AWaveManager::EnsureInfiniteBuilder()
{
	if ( !InfiniteConfig )
	{
		InfiniteBuilder_ = nullptr;
		return;
	}

	if ( !InfiniteBuilder_ )
	{
		InfiniteBuilder_ = NewObject<UInfiniteWaveBuilder>( this );
	}
	const int32 seed = ( InfiniteSessionSeed != 0 ) ? InfiniteSessionSeed : FMath::Rand();
	InfiniteBuilder_->Initialize( InfiniteConfig, seed );
}

void AWaveManager::SetInfiniteConfig( UInfiniteModeConfig* newConfig )
{
	InfiniteConfig = newConfig;
	EnsureInfiniteBuilder();
}

UWaveData* AWaveManager::BuildAndCacheInfiniteWave( int32 waveIndex )
{
	if ( !InfiniteConfig )
	{
		return nullptr;
	}
	EnsureInfiniteBuilder();
	if ( !InfiniteBuilder_ )
	{
		return nullptr;
	}

	UWaveData* generated = InfiniteBuilder_->BuildWave( waveIndex, this );
	if ( generated )
	{
		SelectedWavePresets_.Add( waveIndex, generated );

		if ( bLogSpawning )
		{
			UE_LOG(
			    LogTemp, Log,
			    TEXT( "WaveManager: Built infinite wave %d. Theme=%s Budget=%d EnemyTypes=%d" ), waveIndex,
			    *InfiniteBuilder_->LastThemeId.ToString(), InfiniteBuilder_->LastBudget,
			    generated->EnemySpawnMap.Num()
			);
		}
	}
	return generated;
}

int32 AWaveManager::ClampWaveIndex( int32 waveIndex ) const
{
	const int32 finiteCount = GetWavesCount();

	if ( HasInfiniteMode() )
	{
		return FMath::Max( 0, waveIndex );
	}

	if ( finiteCount == 0 )
	{
		return INDEX_NONE;
	}

	return FMath::Clamp( waveIndex, 0, finiteCount - 1 );
}

void AWaveManager::StartWaves()
{
	const bool bHasFinite = WaveConfig_ && WaveConfig_->Waves.Num() > 0;
	if ( !bHasFinite && !HasInfiniteMode() )
	{
		UE_LOG( LogTemp, Warning, TEXT( "WaveManager: No waves to start." ) );
		return;
	}

	if ( bHasFinite )
	{
		BuildSelectedWavePresetCache();
	}
	EnsureInfiniteBuilder();

	if ( WaveConfig_ && SelectedWavePresets_.Num() == 0 )
	{
		UE_LOG( LogTemp, Warning, TEXT( "WaveManager: Selected preset cache is empty, building now." ) );
	}

	const int32 StartIndex = ClampWaveIndex( CurrentWaveIndex );
	if ( StartIndex == INDEX_NONE )
	{
		UE_LOG( LogTemp, Warning, TEXT( "WaveManager: No waves to start." ) );
		return;
	}
	UE_LOG(
	    LogTemp, Warning, TEXT( "WaveManager: Using WaveConfig=%s, Infinite=%s" ), *GetNameSafe( WaveConfig_ ),
	    HasInfiniteMode() ? TEXT( "yes" ) : TEXT( "no" )
	);

	bHasRequestedFirstWave_ = true;
	StartWaveAtIndex( StartIndex );
}

void AWaveManager::StartWaveAtIndex( int32 waveIndex )
{
	if ( !WaveConfig_ && !HasInfiniteMode() )
	{
		return;
	}

	if ( WaveConfig_ && SelectedWavePresets_.Num() == 0 )
	{
		UE_LOG( LogTemp, Warning, TEXT( "WaveManager: Selected preset cache is empty, building now." ) );
	}

	const int32 clampedIndex = ClampWaveIndex( waveIndex );
	bHasRequestedFirstWave_ = true;

	if ( clampedIndex == INDEX_NONE )
	{
		UE_LOG( LogTemp, Warning, TEXT( "WaveManager: StartWaveAtIndex invalid index %d" ), waveIndex );
		return;
	}

	if ( bIsWaveActive_ )
	{
		CancelCurrentWave();
	}

	CurrentWaveIndex = clampedIndex;

	if ( !bEndlessRunStarted_ && IsInfiniteWaveIndex( CurrentWaveIndex ) )
	{
		bEndlessRunStarted_ = true;
		UE_LOG( LogTemp, Log, TEXT( "WaveManager: endless run started at wave %d." ), CurrentWaveIndex );
	}

	const UWaveData* WaveData = GetSelectedWaveData( CurrentWaveIndex );
	if ( !WaveData )
	{
		UE_LOG( LogTemp, Warning, TEXT( "WaveManager: Wave %d has no valid preset." ), CurrentWaveIndex );
		if ( IsInfiniteWaveIndex( CurrentWaveIndex ) )
		{
			BroadcastAllWavesCompleted();
			return;
		}
		MoveToNextWaveAndStart();
		return;
	}

	RemainingEnemiesPerClass_ = GetNextWaveComposition( CurrentWaveIndex );
	bIsWaveActive_ = true;
	OnWaveStarted.Broadcast( CurrentWaveIndex );

	if ( bLogSpawning )
	{
		UE_LOG(
		    LogTemp, Log, TEXT( "WaveManager: Starting wave %d (preset=%s)" ), CurrentWaveIndex,
		    *GetNameSafe( WaveData )
		);
	}

	ScheduleWaveSpawns( WaveData, CurrentWaveIndex );

	if ( GetWorld() )
	{
		GetWorld()->GetTimerManager().ClearTimer( WaveEndTimerHandle_ );

		FTimerDelegate endDelegate;
		endDelegate.BindUFunction( this, FName( "OnWaveEndTimerElapsed" ), CurrentWaveIndex );

		if ( const UCoreManager* core = GetGameInstance()->GetSubsystem<UCoreManager>() )
		{
			if ( const UGameLoopManager* loopManager = core->GetGameLoop() )
			{
				GetWorld()->GetTimerManager().SetTimer(
				    WaveEndTimerHandle_, endDelegate, loopManager->GetCombatTotalTime(), false
				);
			}
		}
	}
}

void AWaveManager::ScheduleWaveSpawns( const UWaveData* waveData, int32 waveIndex )
{
	if ( !GetWorld() || !waveData )
	{
		return;
	}

	ClearActiveTimers();

	for ( const TPair<TSubclassOf<AUnit>, FEnemySpawnSettings>& pair : waveData->EnemySpawnMap )
	{
		const TSubclassOf<AUnit>& enemyClass = pair.Key;
		const FEnemySpawnSettings& spawnSettings = pair.Value;

		if ( !enemyClass )
		{
			continue;
		}

		for ( const FPortalSpawnEntry& portalEntry : spawnSettings.Portals )
		{
			if ( portalEntry.Count <= 0 || portalEntry.SpawnPointId.IsNone() )
			{
				continue;
			}

			for ( int32 enemyIndex = 0; enemyIndex < portalEntry.Count; ++enemyIndex )
			{
				const float timeFromWaveStart =
				    portalEntry.StartDelay + ( enemyIndex * portalEntry.SpawnInterval ) + 0.1f; // DO NOT REMOVE

				if ( timeFromWaveStart < 0.f )
				{
					continue;
				}

				FTimerDelegate spawnDelegate;
				spawnDelegate.BindUFunction(
				    this, FName( "SpawnEnemy" ), waveIndex, enemyClass, portalEntry.SpawnPointId, enemyIndex
				);

				FTimerHandle timerHandle;
				GetWorld()->GetTimerManager().SetTimer( timerHandle, spawnDelegate, timeFromWaveStart, false );

				ActiveSpawnTimers_.Add( timerHandle );
			}
		}
	}
}

void AWaveManager::SpawnEnemy( int32 waveIndex, UClass* enemyClass, FName spawnPointId, int32 enemyIndex )
{
	if ( !GetWorld() || !enemyClass )
	{
		return;
	}

	if ( waveIndex != CurrentWaveIndex || !bIsWaveActive_ )
	{
		return;
	}

	const bool bIsInfinite = IsInfiniteWaveIndex( waveIndex );
	if ( !bIsInfinite && ( !WaveConfig_ || !WaveConfig_->Waves.IsValidIndex( waveIndex ) ) )
	{
		return;
	}

	const UWaveData* waveData = GetSelectedWaveData( CurrentWaveIndex );
	if ( !waveData )
	{
		return;
	}

	const FEnemySpawnSettings* spawnSettings = waveData->EnemySpawnMap.Find( enemyClass );
	if ( !spawnSettings )
	{
		return;
	}

	const AEnemyGroupSpawnPoint* spawnPointConst = AEnemyGroupSpawnPoint::FindSpawnPointById( this, spawnPointId );
	if ( !spawnPointConst )
	{
		if ( bLogSpawning )
		{
			UE_LOG(
			    LogTemp, Warning,
			    TEXT( "WaveManager: Spawn point '%s' not found for enemy %s (wave=%d, spawnIndex=%d)" ),
			    *spawnPointId.ToString(), *GetNameSafe( enemyClass ), waveIndex, enemyIndex
			);
		}
		return;
	}

	FTransform spawnTransform = spawnPointConst->GetActorTransform();

	float capsuleRadius = 34.f;
	float capsuleHalfHeight = 88.f;

	if ( AUnit* defaultUnit = Cast<AUnit>( enemyClass->GetDefaultObject() ) )
	{
		if ( UCapsuleComponent* comp = defaultUnit->FindComponentByClass<UCapsuleComponent>() )
		{
			capsuleRadius = comp->GetUnscaledCapsuleRadius();
			capsuleHalfHeight = comp->GetUnscaledCapsuleHalfHeight();
		}
	}

	auto* unitBuilder = NewObject<UUnitBuilder>( this );

	FTransform finalTransform = unitBuilder->FindNonOverlappingSpawnTransform(
	    spawnTransform, capsuleRadius, capsuleHalfHeight, 400.f, 24,
	    /*bProjectToNavMesh=*/false
	);

	unitBuilder->CreateNewUnit( enemyClass, finalTransform, this, GetInstigator() );
	unitBuilder->ApplyBuff( &spawnSettings->Buff );
	TWeakObjectPtr<AUnit> spawned = unitBuilder->SpawnUnitAndFinish();

	if ( !spawned.IsValid() || spawned->IsActorBeingDestroyed() )
	{
		if ( bLogSpawning )
		{
			UE_LOG(
			    LogTemp, Warning,
			    TEXT( "WaveManager: Failed to spawn actor for Wave[%d] Enemy[%s] Portal[%s] SpawnIndex[%d]" ),
			    waveIndex, *GetNameSafe( enemyClass ), *spawnPointId.ToString(), enemyIndex
			);
		}
		return;
	}

	SpawnedUnits_.Add( spawned );
	spawned->OnDestroyed.AddDynamic( this, &AWaveManager::HandleSpawnedDestroyed );

	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( AWavePortalManager* portalManager = core->GetWavePortalManager() )
		{
			portalManager->NotifyEnemySpawnStarted( spawnPointId );
		}
	}

	if ( bLogSpawning )
	{
		UE_LOG(
		    LogTemp, Warning, TEXT( "WaveManager: Spawned %s at Wave[%d] Enemy[%s] Portal[%s] SpawnIndex[%d]" ),
		    *spawned->GetName(), waveIndex, *GetNameSafe( enemyClass ), *spawnPointId.ToString(), enemyIndex
		);
	}
}

bool AWaveManager::MoveToNextWaveAndStart()
{
	const bool bHasFinite = WaveConfig_ && GetWavesCount() > 0;
	if ( !bHasFinite && !HasInfiniteMode() )
	{
		return false;
	}

	const int32 nextIndex = CurrentWaveIndex + 1;

	const bool bFiniteHasNext = bHasFinite && WaveConfig_->Waves.IsValidIndex( nextIndex );
	const bool bInfiniteHasNext = IsInfiniteWaveIndex( nextIndex );

	if ( !bFiniteHasNext && !bInfiniteHasNext )
	{
		BroadcastAllWavesCompleted();
		if ( bLogSpawning )
		{
			UE_LOG( LogTemp, Log, TEXT( "WaveManager: No more waves to start." ) );
		}
		return false;
	}

	StartWaveAtIndex( nextIndex );
	return true;
}

void AWaveManager::OnWaveEndTimerElapsed( int32 waveIndex )
{
	if ( waveIndex != CurrentWaveIndex )
	{
		return;
	}

	bIsWaveActive_ = false;
	ClearActiveTimers();

	RemainingEnemiesPerClass_.Empty();

	if ( UCardVisualSubsystem* visuals = UCardVisualSubsystem::Get( this ) )
	{
		visuals->EndAllSticky();
	}

	OnWaveEnded.Broadcast( waveIndex );

	if ( bLogSpawning )
	{
		UE_LOG( LogTemp, Log, TEXT( "WaveManager::OnWaveEndTimerElapsed: Wave %d ended." ), waveIndex );
	}

	if ( GetWavesCount() > 0 && waveIndex >= GetWavesCount() - 1 && !IsInfiniteWaveIndex( waveIndex + 1 ) )
	{
		BroadcastAllWavesCompleted();

		if ( bLogSpawning )
		{
			UE_LOG( LogTemp, Log, TEXT( "WaveManager: All waves completed." ) );
		}
	}
}

void AWaveManager::AdvanceToNextWave()
{
	if ( bIsWaveActive_ )
	{
		CancelCurrentWave();
	}

	const bool bHasFinite = WaveConfig_ && GetWavesCount() > 0;
	if ( !bHasFinite && !HasInfiniteMode() )
	{
		BroadcastAllWavesCompleted();
		if ( bLogSpawning )
		{
			UE_LOG( LogTemp, Log, TEXT( "WaveManager: AdvanceToNextWave called but no waves exist." ) );
		}
		return;
	}

	const int32 nextIndex = CurrentWaveIndex + 1;
	const bool bFiniteHasNext = bHasFinite && WaveConfig_->Waves.IsValidIndex( nextIndex );
	if ( !bFiniteHasNext && !IsInfiniteWaveIndex( nextIndex ) )
	{
		BroadcastAllWavesCompleted();
		if ( bLogSpawning )
		{
			UE_LOG( LogTemp, Log, TEXT( "WaveManager: AdvanceToNextWave called but no more waves." ) );
		}
		return;
	}

	StartWaveAtIndex( nextIndex );
}

void AWaveManager::CancelCurrentWave()
{
	ClearActiveTimers();

	int32 destroyedAmount = DestroyAllEnemies();
	if ( bLogSpawning )
	{
		UE_LOG( LogTemp, Log, TEXT( "WaveManager: Destroyed %d enemies." ), destroyedAmount );
	}

	bIsWaveActive_ = false;

	RemainingEnemiesPerClass_.Empty();

	if ( UCardVisualSubsystem* visuals = UCardVisualSubsystem::Get( this ) )
	{
		visuals->EndAllSticky();
	}

	OnWaveEnded.Broadcast( CurrentWaveIndex );

	if ( bLogSpawning )
	{
		UE_LOG( LogTemp, Log, TEXT( "WaveManager: Current wave cancelled." ) );
	}

}

void AWaveManager::RestartWaves()
{
	SelectedWavePresets_.Empty();
	CancelCurrentWave();
	CurrentWaveIndex = 0;
	if ( InfiniteBuilder_ )
	{
		InfiniteBuilder_->ResetState();
	}
	StartWaves();
}

void AWaveManager::ClearActiveTimers()
{
	if ( !GetWorld() )
	{
		ActiveSpawnTimers_.Reset();
		return;
	}

	FTimerManager& timerManager = GetWorld()->GetTimerManager();
	for ( FTimerHandle& timerHandle : ActiveSpawnTimers_ )
	{
		if ( timerHandle.IsValid() )
		{
			timerManager.ClearTimer( timerHandle );
		}
	}

	ActiveSpawnTimers_.Reset();

	if ( WaveEndTimerHandle_.IsValid() )
	{
		timerManager.ClearTimer( WaveEndTimerHandle_ );
		WaveEndTimerHandle_.Invalidate();
	}
}

bool AWaveManager::SubscribeToAllWavesCompleted( UObject* listener, FName functionName )
{
	if ( !listener || functionName.IsNone() )
	{
		UE_LOG( LogTemp, Warning, TEXT( "WaveManager: SubscribeToAllWavesCompleted invalid args" ) );
		return false;
	}

	// Avoid duplicate subscription from same object
	if ( AllWavesCompletedSubscribers_.Contains( listener ) )
	{
		UE_LOG(
		    LogTemp, Verbose,
		    TEXT(
		        "WaveManager: SubscribeToAllWavesCompleted - already "
		        "subscribed: %s"
		    ),
		    *GetNameSafe( listener )
		);
		return false;
	}

	// Bind script delegate to listener's function and add to multicast
	FScriptDelegate scriptDel;
	scriptDel.BindUFunction( listener, functionName );
	OnAllWavesCompleted.Add( scriptDel );

	AllWavesCompletedSubscribers_.Add( listener );

	UE_LOG(
	    LogTemp, Log, TEXT( "WaveManager: SubscribeToAllWavesCompleted - subscribed %s.%s" ), *GetNameSafe( listener ),
	    *functionName.ToString()
	);
	return true;
}

bool AWaveManager::UnsubscribeFromAllWavesCompleted( UObject* listener, FName functionName )
{
	if ( !listener || functionName.IsNone() )
	{
		UE_LOG( LogTemp, Warning, TEXT( "WaveManager: UnsubscribeFromAllWavesCompleted invalid args" ) );
		return false;
	}

	FScriptDelegate scriptDel;
	scriptDel.BindUFunction( listener, functionName );
	OnAllWavesCompleted.Remove( scriptDel );

	AllWavesCompletedSubscribers_.Remove( listener );

	UE_LOG(
	    LogTemp, Log, TEXT( "WaveManager: UnsubscribeFromAllWavesCompleted - unsubscribed %s.%s" ),
	    *GetNameSafe( listener ), *functionName.ToString()
	);
	return true;
}

void AWaveManager::BroadcastAllWavesCompleted()
{
	// Make sure we only broadcast once per run (until RestartWaves is called)
	if ( bHasBroadcastedAllWavesCompleted_ )
	{
		UE_LOG(
		    LogTemp, Verbose,
		    TEXT(
		        "WaveManager: BroadcastAllWavesCompleted - already "
		        "broadcasted, skipping."
		    )
		);
		return;
	}

	bHasBroadcastedAllWavesCompleted_ = true;

	// Actual broadcast
	OnAllWavesCompleted.Broadcast();

	if ( bLogSpawning )
	{
		UE_LOG( LogTemp, Log, TEXT( "WaveManager: All waves completed (BroadcastAllWavesCompleted)" ) );
	}
}

int32 AWaveManager::DestroyAllEnemies()
{
	int32 destroyed = 0;
	for ( int32 i = SpawnedUnits_.Num() - 1; i >= 0; --i )
	{
		AUnit* unit = SpawnedUnits_[i].Get();
		if ( unit )
		{
			unit->Destroy();
			++destroyed;
		}
	}
	SpawnedUnits_.Empty();

	return destroyed;
}

void AWaveManager::HandleSpawnedDestroyed( AActor* destroyedActor )
{
	if ( IsValid( destroyedActor ) )
	{
		for ( TPair<TSubclassOf<AUnit>, int32>& pair : RemainingEnemiesPerClass_ )
		{
			if ( destroyedActor->IsA( pair.Key ) )
			{
				pair.Value = FMath::Max( 0, pair.Value - 1 );
				break;
			}
		}
		OnWaveEnemiesUpdated.Broadcast();
	}
	for ( int32 i = SpawnedUnits_.Num() - 1; i >= 0; --i )
	{
		if ( SpawnedUnits_[i].Get() == destroyedActor )
		{
			SpawnedUnits_.RemoveAtSwap( i );
			break;
		}
	}

	if ( SpawnedUnits_.IsEmpty() )
	{
		bool bHasPendingSpawns = false;
		if ( bIsWaveActive_ )
		{
			if ( UWorld* world = GetWorld() )
			{
				FTimerManager& tm = world->GetTimerManager();
				for ( const FTimerHandle& h : ActiveSpawnTimers_ )
				{
					if ( tm.IsTimerActive( h ) )
					{
						bHasPendingSpawns = true;
						break;
					}
				}
			}
		}

		if ( !bHasPendingSpawns )
		{
			if ( bIsWaveActive_ )
			{
				bIsWaveActive_ = false;
				ClearActiveTimers();
				if ( UCardVisualSubsystem* visuals = UCardVisualSubsystem::Get( this ) )
				{
					visuals->EndAllSticky();
				}
				OnWaveEnded.Broadcast( CurrentWaveIndex );
			}
			if ( UWorld* world = GetWorld() )
			{
				FTimerDelegate del =
				    FTimerDelegate::CreateUObject( this, &AWaveManager::OnWaveEndTimerElapsed, CurrentWaveIndex );
				world->GetTimerManager().SetTimer( WaveEndTimerHandle_, del, TIME_TO_END_WAVE_AFTER_LAST_DEATH, false );
				OnWaveEndScheduled.Broadcast( TIME_TO_END_WAVE_AFTER_LAST_DEATH );
			}
		}
	}
}

TMap<TSubclassOf<AUnit>, int32> AWaveManager::GetNextWaveComposition( int32 targetWaveIndex ) const
{
	TMap<TSubclassOf<AUnit>, int32> result;

	const bool bIsInfinite = IsInfiniteWaveIndex( targetWaveIndex );
	if ( !bIsInfinite && ( !WaveConfig_ || !WaveConfig_->Waves.IsValidIndex( targetWaveIndex ) ) )
	{
		return result;
	}

	const UWaveData* waveData = GetSelectedWaveData( targetWaveIndex );
	if ( !waveData )
	{
		return result;
	}

	for ( const TPair<TSubclassOf<AUnit>, FEnemySpawnSettings>& pair : waveData->EnemySpawnMap )
	{
		int32 totalCount = 0;
		for ( const FPortalSpawnEntry& portalEntry : pair.Value.Portals )
		{
			totalCount += FMath::Max( 0, portalEntry.Count );
		}

		if ( pair.Key && totalCount > 0 )
		{
			result.FindOrAdd( pair.Key ) += totalCount;
		}
	}

	return result;
}

void AWaveManager::ApplyWaveConfig()
{
	if ( !WaveConfig_ )
	{
		UE_LOG( LogTemp, Warning, TEXT( "WaveManager::ApplyWaveConfig: WaveConfig is null" ) );
		return;
	}

	RuntimeWaveEndSafetyMargin_ = WaveConfig_->WaveEndSafetyMargin;
	bHasBroadcastedAllWavesCompleted_ = false;

	if ( bLogSpawning )
	{
		UE_LOG(
		    LogTemp, Log, TEXT( "WaveManager::ApplyWaveConfig: Applied config '%s' => Waves=%d, SafetyMargin=%.2f" ),
		    *GetNameSafe( WaveConfig_ ), WaveConfig_->Waves.Num(), RuntimeWaveEndSafetyMargin_
		);
	}
}
// Apply to HotSwap wave config (for difficulty and etc)
void AWaveManager::SetWaveConfig( UWaveConfigData* newConfig )
{
	if ( !newConfig )
	{
		UE_LOG( LogTemp, Warning, TEXT( "SetWaveConfig called with nullptr" ) );
		return;
	}

	if ( WaveConfig_ == newConfig )
	{
		return;
	}

	const bool bWasWaveActive = bIsWaveActive_;
	const int32 prevWaveIndex = CurrentWaveIndex;

	CancelCurrentWave();

	WaveConfig_ = newConfig;

	ApplyWaveConfig();

	if ( GetWavesCount() == 0 )
	{
		UE_LOG( LogTemp, Warning, TEXT( "WaveConfig contains no waves" ) );
		return;
	}

	CurrentWaveIndex = FMath::Clamp( prevWaveIndex, 0, GetWavesCount() - 1 );

	UE_LOG( LogTemp, Log, TEXT( "WaveConfig swapped. Continuing from wave %d" ), CurrentWaveIndex );

	if ( bWasWaveActive )
	{
		StartWaveAtIndex( CurrentWaveIndex );
	}
}

#if WITH_EDITOR
void AWaveManager::PostEditChangeProperty( FPropertyChangedEvent& propertyChangedEvent )
{
	Super::PostEditChangeProperty( propertyChangedEvent );

	const FName propertyName = propertyChangedEvent.Property ? propertyChangedEvent.Property->GetFName() : NAME_None;

	if ( propertyName == GET_MEMBER_NAME_CHECKED( AWaveManager, WaveConfig_ ) )
	{
		if ( WaveConfig_ )
		{
			ApplyWaveConfig();
		}
	}
}
#endif
const UWaveData* AWaveManager::GetWaveData( int32 index ) const
{
	if ( const TObjectPtr<UWaveData>* found = SelectedWavePresets_.Find( index ) )
	{
		return found->Get();
	}

	if ( !WaveConfig_ || !WaveConfig_->Waves.IsValidIndex( index ) )
	{
		return nullptr;
	}
	return PickWeightedWavePreset( WaveConfig_->Waves[index] );
}

int32 AWaveManager::GetWavesCount() const
{
    return WaveConfig_ ? WaveConfig_->Waves.Num() : 0;
}

const FEnemyBuff* AWaveManager::FindBuffForCurrentWave( TSubclassOf<AUnit> enemyClass ) const
{
	const UWaveData* waveData = GetWaveData( CurrentWaveIndex );
	if ( !waveData || !enemyClass )
	{
		return nullptr;
	}

	const FEnemySpawnSettings* spawnSettings = waveData->EnemySpawnMap.Find( enemyClass );
	return spawnSettings ? &spawnSettings->Buff : nullptr;
}

void AWaveManager::ClearWavePresetCache()
{
	SelectedWavePresets_.Empty();
}

const UWaveData* AWaveManager::PickWeightedWavePreset( const FWavePresetSlot& slot ) const
{
	float totalWeight = 0.f;

	for ( const FWeightedWavePreset& entry : slot.Presets )
	{
		if ( entry.Preset && entry.Weight > 0.f )
		{
			totalWeight += entry.Weight;
		}
	}

	if ( totalWeight <= 0.f )
	{
		for ( const FWeightedWavePreset& entry : slot.Presets )
		{
			if ( entry.Preset )
			{
				return entry.Preset.Get();
			}
		}
		return nullptr;
	}

	const float roll = FMath::FRandRange( 0.f, totalWeight );
	float accumulated = 0.f;

	for ( const FWeightedWavePreset& entry : slot.Presets )
	{
		if ( !entry.Preset || entry.Weight <= 0.f )
		{
			continue;
		}

		accumulated += entry.Weight;
		if ( roll <= accumulated )
		{
			return entry.Preset.Get();
		}
	}

	for ( const FWeightedWavePreset& entry : slot.Presets )
	{
		if ( entry.Preset )
		{
			return entry.Preset.Get();
		}
	}

	return nullptr;
}

void AWaveManager::BuildSelectedWavePresetCache()
{
	SelectedWavePresets_.Empty();

	if ( !WaveConfig_ )
	{
		UE_LOG( LogTemp, Warning, TEXT( "WaveManager: WaveConfig_ is NULL" ) );
		return;
	}

	UE_LOG(
	    LogTemp, Warning, TEXT( "WaveManager: WaveConfig = %s, Waves=%d" ), *GetNameSafe( WaveConfig_ ),
	    WaveConfig_->Waves.Num()
	);

	for ( int32 waveIndex = 0; waveIndex < WaveConfig_->Waves.Num(); ++waveIndex )
	{
		const FWavePresetSlot& slot = WaveConfig_->Waves[waveIndex];

		UE_LOG( LogTemp, Warning, TEXT( "WaveManager: Wave[%d] presets=%d" ), waveIndex, slot.Presets.Num() );

		for ( int32 i = 0; i < slot.Presets.Num(); ++i )
		{
			const FWeightedWavePreset& entry = slot.Presets[i];
			UE_LOG(
			    LogTemp, Warning, TEXT( "  preset[%d]=%s weight=%.3f valid=%d" ), i, *GetNameSafe( entry.Preset ),
			    entry.Weight, entry.Preset ? 1 : 0
			);
		}

		const UWaveData* chosen = PickWeightedWavePreset( slot );
		UE_LOG( LogTemp, Warning, TEXT( "WaveManager: Wave[%d] chosen=%s" ), waveIndex, *GetNameSafe( chosen ) );

		if ( chosen )
		{
			SelectedWavePresets_.Add( waveIndex, const_cast<UWaveData*>( chosen ) );
		}
	}
}

const UWaveData* AWaveManager::GetSelectedWaveData( int32 waveIndex ) const
{
	if ( const TObjectPtr<UWaveData>* found = SelectedWavePresets_.Find( waveIndex ) )
	{
		return found->Get();
	}

	if ( IsInfiniteWaveIndex( waveIndex ) )
	{
		return const_cast<AWaveManager*>( this )->BuildAndCacheInfiniteWave( waveIndex );
	}

	return nullptr;
}