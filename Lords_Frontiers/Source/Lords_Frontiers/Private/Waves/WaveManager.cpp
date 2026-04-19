#include "Lords_Frontiers/Public/Waves/WaveManager.h"

#include "AI/Path/Path.h"
#include "Core/CoreManager.h"
#include "Core/GameLoop/GameLoopManager.h"
#include "Lords_Frontiers/Public/Waves/WaveData.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#if WITH_EDITOR
#include "Editor.h"
#endif

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
	}
	if ( bAutoStartOnBeginPlay )
	{
		StartWaves();
	}
}

int32 AWaveManager::ClampWaveIndex( int32 waveIndex ) const
{
	if ( GetWavesCount() == 0 )
	{
		return INDEX_NONE;
	}

	return FMath::Clamp( waveIndex, 0, GetWavesCount() - 1 );
}

void AWaveManager::StartWaves()
{
	if ( !WaveConfig_ || GetWavesCount() == 0 )
	{
		UE_LOG( LogTemp, Warning, TEXT( "WaveManager: No waves to start." ) );
		return;
	}

	const int32 startIndex = ClampWaveIndex( CurrentWaveIndex );
	if ( startIndex == INDEX_NONE )
	{
		UE_LOG( LogTemp, Warning, TEXT( "WaveManager: No waves to start." ) );
		return;
	}

	bHasRequestedFirstWave_ = true;

	StartWaveAtIndex( startIndex );
}

void AWaveManager::StartWaveAtIndex( int32 waveIndex )
{
	if ( !WaveConfig_ )
	{
		return;
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

	const UWaveData* WaveData = GetWaveData( CurrentWaveIndex );
	if ( !WaveData )
	{
		UE_LOG( LogTemp, Warning, TEXT( "WaveManager: Wave %d is null. Skipping." ), CurrentWaveIndex );
		MoveToNextWaveAndStart();
		return;
	}

	if ( WaveData->EnemySpawnMap.Num() == 0 )
	{
		UE_LOG( LogTemp, Warning, TEXT( "WaveManager: Wave %d has no enemies. Skipping." ), CurrentWaveIndex );
		MoveToNextWaveAndStart();
		return;
	}

	bIsWaveActive_ = true;
	OnWaveStarted.Broadcast( CurrentWaveIndex );

	if ( bLogSpawning )
	{
		UE_LOG( LogTemp, Log, TEXT( "WaveManager: Starting wave %d" ), CurrentWaveIndex );
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

void AWaveManager::ScheduleWaveSpawns( const UWaveData* WaveData, int32 waveIndex )
{
	if ( !GetWorld() || !WaveData )
	{
		return;
	}

	ClearActiveTimers();

	TArray<TSubclassOf<AUnit>> EnemyClasses;
	WaveData->EnemySpawnMap.GetKeys( EnemyClasses );

	EnemyClasses.Sort( []( const TSubclassOf<AUnit>& A, const TSubclassOf<AUnit>& B )
	                   { return GetNameSafe( A.Get() ) < GetNameSafe( B.Get() ); } );

	for ( const TSubclassOf<AUnit>& EnemyClassKey : EnemyClasses )
	{
		const UClass* EnemyClass = EnemyClassKey.Get();
		if ( !EnemyClass )
		{
			continue;
		}

		const FEnemySpawnSettings* SpawnSettings = WaveData->EnemySpawnMap.Find( EnemyClassKey );
		if ( !SpawnSettings )
		{
			continue;
		}

		for ( const FPortalSpawnEntry& PortalEntry : SpawnSettings->Portals )
		{
			if ( PortalEntry.Count <= 0 || PortalEntry.SpawnPointId.IsNone() )
			{
				continue;
			}

			for ( int32 enemyIndex = 0; enemyIndex < PortalEntry.Count; ++enemyIndex )
			{
				const float timeFromWaveStart =
				    SpawnSettings->StartDelay + ( enemyIndex * SpawnSettings->SpawnInterval );

				if ( timeFromWaveStart < 0.f )
				{
					continue;
				}

				FTimerDelegate spawnDelegate;
				spawnDelegate.BindUFunction(
				    this, FName( "SpawnEnemy" ), waveIndex, const_cast<UClass*>( EnemyClass ), PortalEntry.SpawnPointId,
				    enemyIndex
				);

				FTimerHandle timerHandle;
				GetWorld()->GetTimerManager().SetTimer( timerHandle, spawnDelegate, timeFromWaveStart, false );

				ActiveSpawnTimers_.Add( timerHandle );

				if ( bLogSpawning )
				{
					UE_LOG(
					    LogTemp, Log,
					    TEXT( "WaveManager: Scheduled spawn Wave[%d] Enemy[%s] Portal[%s] SpawnIndex[%d] at +%f s" ),
					    waveIndex, *GetNameSafe( EnemyClass ), *PortalEntry.SpawnPointId.ToString(), enemyIndex,
					    timeFromWaveStart
					);
				}
			}
		}
	}
}

void AWaveManager::SpawnEnemy( int32 waveIndex, UClass* EnemyClass, FName SpawnPointId, int32 enemyIndex )
{
	if ( !GetWorld() || !WaveConfig_ || !EnemyClass )
	{
		return;
	}

	if ( waveIndex != CurrentWaveIndex || !bIsWaveActive_ )
	{
		return;
	}

	if ( !WaveConfig_->Waves.IsValidIndex( waveIndex ) )
	{
		return;
	}

	const UWaveData* WaveData = WaveConfig_->Waves[waveIndex];
	if ( !WaveData )
	{
		return;
	}

	const FEnemySpawnSettings* SpawnSettings = WaveData->EnemySpawnMap.Find( EnemyClass );
	if ( !SpawnSettings )
	{
		return;
	}

	const AEnemyGroupSpawnPoint* SpawnPoint = nullptr;
	{
		TArray<AActor*> FoundSpawnPoints;
		UGameplayStatics::GetAllActorsOfClass( GetWorld(), AEnemyGroupSpawnPoint::StaticClass(), FoundSpawnPoints );

		for ( AActor* Actor : FoundSpawnPoints )
		{
			if ( AEnemyGroupSpawnPoint* SP = Cast<AEnemyGroupSpawnPoint>( Actor ) )
			{
				if ( SP->SpawnPointId == SpawnPointId )
				{
					SpawnPoint = SP;
					break;
				}
			}
		}
	}

	if ( !SpawnPoint )
	{
		if ( bLogSpawning )
		{
			UE_LOG(
			    LogTemp, Warning,
			    TEXT( "WaveManager: Spawn point '%s' not found for enemy %s (wave=%d, spawnIndex=%d)" ),
			    *SpawnPointId.ToString(), *GetNameSafe( EnemyClass ), waveIndex, enemyIndex
			);
		}
		return;
	}

	FTransform spawnTransform = SpawnPoint->GetActorTransform();

	float capsuleRadius = 34.f;
	float capsuleHalfHeight = 88.f;

	if ( AUnit* defaultUnit = Cast<AUnit>( EnemyClass->GetDefaultObject() ) )
	{
		if ( UCapsuleComponent* C = defaultUnit->FindComponentByClass<UCapsuleComponent>() )
		{
			capsuleRadius = C->GetUnscaledCapsuleRadius();
			capsuleHalfHeight = C->GetUnscaledCapsuleHalfHeight();
		}
	}

	FTransform FinalTransform = FindNonOverlappingSpawnTransform(
	    spawnTransform, capsuleRadius, capsuleHalfHeight, 400.f, 24, /*bProjectToNavMesh=*/false
	);

	AUnit* spawned = GetWorld()->SpawnActorDeferred<AUnit>( EnemyClass, FinalTransform, this, GetInstigator() );

	if ( !spawned )
	{
		if ( bLogSpawning )
		{
			UE_LOG(
			    LogTemp, Warning,
			    TEXT( "WaveManager: Failed to spawn actor for Wave[%d] Enemy[%s] Portal[%s] SpawnIndex[%d]" ),
			    waveIndex, *GetNameSafe( EnemyClass ), *SpawnPointId.ToString(), enemyIndex
			);
		}
		return;
	}

	if ( FEnemyBuff* buff = EnemyBuffs.Find( TSubclassOf<AUnit>( EnemyClass ) ) )
	{
		spawned->ChangeStats( buff );
	}

	UGameplayStatics::FinishSpawningActor( spawned, FinalTransform );

	if ( !IsValid( spawned ) || spawned->IsActorBeingDestroyed() )
	{
		if ( bLogSpawning )
		{
			UE_LOG(
			    LogTemp, Warning,
			    TEXT( "WaveManager: Spawn failed / actor invalid for Wave[%d] Enemy[%s] Portal[%s] SpawnIndex[%d]" ),
			    waveIndex, *GetNameSafe( EnemyClass ), *SpawnPointId.ToString(), enemyIndex
			);
		}
		return;
	}

	SpawnedUnits_.Add( spawned );
	spawned->OnDestroyed.AddDynamic( this, &AWaveManager::HandleSpawnedDestroyed );

	if ( bLogSpawning )
	{
		UE_LOG(
		    LogTemp, Warning, TEXT( "WaveManager: Spawned %s at Wave[%d] Enemy[%s] Portal[%s] SpawnIndex[%d]" ),
		    *spawned->GetName(), waveIndex, *GetNameSafe( EnemyClass ), *SpawnPointId.ToString(), enemyIndex
		);
	}
}

bool AWaveManager::MoveToNextWaveAndStart()
{
	if ( !WaveConfig_ || GetWavesCount() == 0 )
	{
		return false;
	}

	const int32 nextIndex = CurrentWaveIndex + 1;

	if ( !WaveConfig_->Waves.IsValidIndex( nextIndex ) )
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

	OnWaveEnded.Broadcast( waveIndex );

	if ( bLogSpawning )
	{
		UE_LOG( LogTemp, Log, TEXT( "WaveManager::OnWaveEndTimerElapsed: Wave %d ended." ), waveIndex );
	}

	if ( GetWavesCount() > 0 && waveIndex >= GetWavesCount() - 1 )
	{
		BroadcastAllWavesCompleted();

		if ( bLogSpawning )
		{
			UE_LOG( LogTemp, Log, TEXT( "WaveManager: All waves completed." ) );
		}
	}

	UpdateSpawnCounts( waveIndex );
}

void AWaveManager::AdvanceToNextWave()
{
	if ( bIsWaveActive_ )
	{
		CancelCurrentWave();
	}

	if ( !WaveConfig_ || GetWavesCount() == 0 )
	{
		BroadcastAllWavesCompleted();
		if ( bLogSpawning )
		{
			UE_LOG( LogTemp, Log, TEXT( "WaveManager: AdvanceToNextWave called but no waves exist." ) );
		}
		return;
	}

	const int32 nextIndex = CurrentWaveIndex + 1;
	if ( !WaveConfig_->Waves.IsValidIndex( nextIndex ) )
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

	OnWaveEnded.Broadcast( CurrentWaveIndex );

	if ( bLogSpawning )
	{
		UE_LOG( LogTemp, Log, TEXT( "WaveManager: Current wave cancelled." ) );
	}

}

void AWaveManager::RestartWaves()
{
	CancelCurrentWave();
	CurrentWaveIndex = 0;
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

//(Artyom)
// find location around enemy group spawnpoint if there is a unit
FTransform AWaveManager::FindNonOverlappingSpawnTransform(
    const FTransform& desiredTransform, float capsuleRadius, float capsuleHalfHeight, float maxSearchRadius,
    int32 maxAttempts, bool bProjectToNavMesh
) const
{
	UWorld* world = GetWorld();
	if ( !world )
	{
		return desiredTransform;
	}

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>( world );

	const FVector origin = desiredTransform.GetLocation();
	const FQuat rotation = desiredTransform.GetRotation();

	// collision query params
	FCollisionQueryParams queryParams( SCENE_QUERY_STAT( SpawnOverlap ), false );
	queryParams.bFindInitialOverlaps = true;
	queryParams.AddIgnoredActor( this );

	const ECollisionChannel channelToTest = ECC_Pawn; // check collisions with pawn

	// try origin first
	{
		FCollisionShape shape = FCollisionShape::MakeCapsule( capsuleRadius, capsuleHalfHeight );
		if ( !world->OverlapAnyTestByChannel( origin, FQuat::Identity, channelToTest, shape, queryParams ) )
		{
#if WITH_EDITOR
			DrawDebugSphere( world, origin, capsuleRadius + 10.f, 8, FColor::Green, false, 3.0f );
#endif
			FTransform Result = desiredTransform;
			Result.SetLocation( origin );
			return Result;
		}
	}

	// spiral sampling
	const float angleStep = FMath::DegreesToRadians( 30.0f );
	const int32 samplesPerRing = FMath::Max( 6, FMath::RoundToInt( 2.f * PI / angleStep ) );
	const float radiusStep = FMath::Max( capsuleRadius * 2.0f, 50.f );

	int32 attempt = 0;
	while ( attempt < maxAttempts )
	{
		const int32 ring = attempt / samplesPerRing + 1;
		const int32 index = attempt % samplesPerRing;
		const float radius = FMath::Min( radiusStep * ring, maxSearchRadius );
		const float angle = angleStep * index;

		const FVector localOffset = FVector( FMath::Cos( angle ) * radius, FMath::Sin( angle ) * radius, 0.f );
		FVector candidate = origin + rotation.RotateVector( localOffset );

		// project to navmesh
		if ( bProjectToNavMesh && NavSys )
		{
			FNavLocation NavLoc;
			if ( NavSys->ProjectPointToNavigation( candidate, NavLoc, FVector( 100.f, 100.f, 300.f ) ) )
			{
				candidate = NavLoc.Location;
			}
		}

		FCollisionShape shape = FCollisionShape::MakeCapsule( capsuleRadius, capsuleHalfHeight );
		bool bOverlaps =
		    world->OverlapAnyTestByChannel( candidate, FQuat::Identity, channelToTest, shape, queryParams );

		if ( !bOverlaps )
		{
#if WITH_EDITOR
			DrawDebugSphere( world, candidate, capsuleRadius + 10.f, 8, FColor::Green, false, 3.0f );
#endif
			FTransform result = desiredTransform;
			result.SetLocation( candidate );
			return result;
		}

		++attempt;
	}

	// not found - return desired
	return desiredTransform;
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
	FScriptDelegate ScriptDel;
	ScriptDel.BindUFunction( listener, functionName );
	OnAllWavesCompleted.Add( ScriptDel );

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

	FScriptDelegate ScriptDel;
	ScriptDel.BindUFunction( listener, functionName );
	OnAllWavesCompleted.Remove( ScriptDel );

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

void AWaveManager::UpdateSpawnCounts( int32 waveIndex )
{
	if ( !WaveConfig_ || !WaveConfig_->Waves.IsValidIndex( waveIndex ) )
	{
		return;
	}

	const UWaveData* WaveData = WaveConfig_->Waves[waveIndex];
	if ( !WaveData )
	{
		return;
	}

	for ( const TPair<TSubclassOf<AUnit>, FEnemySpawnSettings>& Pair : WaveData->EnemySpawnMap )
	{
		const TSubclassOf<AUnit>& EnemyClass = Pair.Key;
		const FEnemySpawnSettings& SpawnSettings = Pair.Value;

		int32 TotalCount = 0;
		for ( const FPortalSpawnEntry& PortalEntry : SpawnSettings.Portals )
		{
			TotalCount += FMath::Max( 0, PortalEntry.Count );
		}

		if ( TotalCount > 0 )
		{
			if ( FEnemyBuff* buff = EnemyBuffs.Find( EnemyClass ) )
			{
				buff->SpawnCount += TotalCount;
			}
		}
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
	// Remove from SpawnedUnits
	for ( int32 i = SpawnedUnits_.Num() - 1; i >= 0; --i )
	{
		if ( SpawnedUnits_[i].Get() == destroyedActor /* || !SpawnedUnits[i].IsValid*/ )
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
			UE_LOG( LogTemp, Log, TEXT( "WaveManager: SpawnedUnits empty, scheduling end-of-wave timer (1s)." ) );

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

	if ( !WaveConfig_ || !WaveConfig_->Waves.IsValidIndex( targetWaveIndex ) )
	{
		return result;
	}

	const UWaveData* WaveData = WaveConfig_->Waves[targetWaveIndex];
	if ( !WaveData )
	{
		return result;
	}

	for ( const TPair<TSubclassOf<AUnit>, FEnemySpawnSettings>& Pair : WaveData->EnemySpawnMap )
	{
		int32 TotalCount = 0;
		for ( const FPortalSpawnEntry& PortalEntry : Pair.Value.Portals )
		{
			TotalCount += FMath::Max( 0, PortalEntry.Count );
		}

		if ( Pair.Key && TotalCount > 0 )
		{
			result.FindOrAdd( Pair.Key ) += TotalCount;
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
	const int32 PrevWaveIndex = CurrentWaveIndex;

	CancelCurrentWave();

	WaveConfig_ = newConfig;

	ApplyWaveConfig();

	if ( GetWavesCount() == 0 )
	{
		UE_LOG( LogTemp, Warning, TEXT( "WaveConfig contains no waves" ) );
		return;
	}

	CurrentWaveIndex = FMath::Clamp( PrevWaveIndex, 0, GetWavesCount() - 1 );

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
const UWaveData* AWaveManager::GetWaveData( int32 Index ) const
{
	if ( !WaveConfig_ || !WaveConfig_->Waves.IsValidIndex( Index ) )
	{
		return nullptr;
	}

	return WaveConfig_->Waves[Index];
}

int32 AWaveManager::GetWavesCount() const
{
    return WaveConfig_ ? WaveConfig_->Waves.Num() : 0;
}