#pragma once

#include "EnemyBuff.h"
#include "Lords_Frontiers/Public/Units/Unit.h"
#include "Lords_Frontiers/Public/Waves/WaveConfig.h"
#include "Lords_Frontiers/Public/Waves/WaveData.h"
#include "Wave.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "WaveManager.generated.h"

class AUnitAIManager;
class AGridManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnWaveStartedSignature, int32, WaveIndex ); // needed to call from BP
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnWaveEndedSignature, int32, WaveIndex );
DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnAllWavesCompletedSignature );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnWaveEndScheduled, float, SecondsRemaining );

/*
 (Artyom)
 WaveManager - actor placed on level to manage sequencing of waves
 */
UCLASS( Blueprintable )
class LORDS_FRONTIERS_API AWaveManager : public AActor
{
	GENERATED_BODY()

public:
	AWaveManager();

	// Start playing waves from CurrentWaveIndex (or first wave if
	// CurrentWaveIndex is invalid).
	UFUNCTION( BlueprintCallable, Category = "Settings|Wave" )
	void StartWaves();

	// Start wave by index (0-based).
	UFUNCTION( BlueprintCallable, Category = "Settings|Wave" )
	void StartWaveAtIndex( int32 waveIndex );

	// Advance to next wave manually (skips waiting). If there are no more waves,
	// broadcasts completion
	UFUNCTION( BlueprintCallable, Category = "Settings|Wave" )
	void AdvanceToNextWave();

	// Stop/cancel current wave (clears timers). Useful for editor or reset
	UFUNCTION( BlueprintCallable, Category = "Settings|Wave" )
	void CancelCurrentWave();

	// Restart waves from the beginning (index 0)
	UFUNCTION( BlueprintCallable, Category = "Settings|Wave" )
	void RestartWaves();

	// Returns whether a wave is currently active (spawning)
	UFUNCTION( BlueprintPure, Category = "Settings|Wave" )
	bool IsWaveActive() const
	{
		return bIsWaveActive_;
	}

	UFUNCTION( BlueprintPure, Category = "Settings|Wave" )
	bool IsFirstWaveRequested() const
	{
		return bHasRequestedFirstWave_;
	}

	
	UPROPERTY( EditAnywhere, Category = "Settings|Wave|Buffs" )
	TMap<TSubclassOf<AUnit>, FEnemyBuff> EnemyBuffs;

	// Current wave index (0-based)
	int32 CurrentWaveIndex = 0;

	// List of waves. Index in array is the wave number
	TArray<FWave> Waves;

	// If true, WaveManager will auto-start the first wave on BeginPlay (if there
	// are waves)
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Wave" )
	bool bAutoStartOnBeginPlay = false;

	// If true, logs debug messages about spawning
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Wave" )
	bool bLogSpawning = true;

	// Delegate: broadcast when a wave starts
	UPROPERTY( BlueprintAssignable, Category = "Settings|Wave|Events" )
	FOnWaveStartedSignature OnWaveStarted;

	// Delegate: broadcast when a wave ends (all groups finished)
	UPROPERTY( BlueprintAssignable, Category = "Settings|Wave|Events" )
	FOnWaveEndedSignature OnWaveEnded;

	// Delegate: broadcast when all waves in Waves[] have been completed
	UPROPERTY( BlueprintAssignable, Category = "Settings|Wave|Events" )
	FOnAllWavesCompletedSignature OnAllWavesCompleted;

	UFUNCTION( BlueprintCallable, Category = "Settings|Wave|Events" )
	bool SubscribeToAllWavesCompleted( UObject* listener, FName functionName );

	// Unsubscribe previously subscribed listener.
	UFUNCTION( BlueprintCallable, Category = "Settings|Wave|Events" )
	bool UnsubscribeFromAllWavesCompleted( UObject* listener, FName functionName );

	// Broadcast "All waves completed" to all listeners (broadcast happens only
	// once).
	UFUNCTION( BlueprintCallable, Category = "Settings|Wave|Events" )
	void BroadcastAllWavesCompleted();

	UPROPERTY( BlueprintAssignable, Category = "Settings|Wave|Events" )
	FOnWaveEndScheduled OnWaveEndScheduled;
  
	UFUNCTION( BlueprintCallable, Category = "Settings|Wave|UI" )
	TMap<TSubclassOf<AUnit>, int32> GetNextWaveComposition( int32 TargetWaveIndex ) const;

	UPROPERTY( EditAnywhere, Category = "Settings|WaveConfig" )
	TObjectPtr<UWaveConfigData> WaveConfig_ = nullptr;

	UFUNCTION( BlueprintCallable, Category = "Wave|Config" )
	void SetWaveConfig( UWaveConfigData* newConfig );

	UFUNCTION( BlueprintCallable, Category = "Wave|Config" )
	void ApplyWaveConfig();

	const UWaveData* GetWaveData( int32 Index ) const;

	int32 GetWavesCount() const;

protected:
	virtual void BeginPlay() override;

	// Internal helper to schedule all spawn timers for a wave
	void ScheduleWaveSpawns( const UWaveData* WaveData, int32 waveIndex );

	UFUNCTION( BlueprintCallable )
	void UpdateSpawnCounts( int32 waveIndex );
	// Schedules spawn for a single enemy (called via timer)
	UFUNCTION()
	void SpawnEnemy( int32 waveIndex, UClass* EnemyClass, FName SpawnPointId, int32 enemyIndex );

	// Called when a scheduled wave-end timer elapses
	UFUNCTION()
	void OnWaveEndTimerElapsed( int32 waveIndex );

	// Clean up timers related to the current wave
	void ClearActiveTimers();

	// Move to next wave index and optionally start it. Returns true if there was
	// a next wave
	bool MoveToNextWaveAndStart();

	// Find spawn point near Enemygroupspawnpoint in case unit is already in it
	FTransform FindNonOverlappingSpawnTransform(
	    const FTransform& desiredTransform, float capsuleRadius, float capsuleHalfHeight, float maxSearchRadius = 600.f,
	    int32 maxAttempts = 32, bool bProjectToNavMesh = false
	) const;

	// Active timer handles (for enemy spawns) so we can clear them if needed
	UPROPERTY( Transient )
	TArray<FTimerHandle> ActiveSpawnTimers_;

	// Active timer handle for wave end
	UPROPERTY( Transient )
	FTimerHandle WaveEndTimerHandle_;

	// True while a wave is active (spawning or waiting for last spawn to finish)
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Wave" )
	bool bIsWaveActive_ = false;

	UPROPERTY( Transient )
	bool bHasRequestedFirstWave_ = false;

	// returns wave index clamped into array bounds (or INDEX_NONE if no waves)
	int32 ClampWaveIndex( int32 waveIndex ) const;

	UPROPERTY( Transient )
	TSet<TWeakObjectPtr<UObject>> AllWavesCompletedSubscribers_;

	// broadcast OnAllWavesCompleted only once (until waves are restarted).
	UPROPERTY( Transient )
	bool bHasBroadcastedAllWavesCompleted_ = false;

	UPROPERTY( Transient )
	TArray<TWeakObjectPtr<AUnit>> SpawnedUnits_;

	int32 DestroyAllEnemies();

	UFUNCTION()
	void HandleSpawnedDestroyed( AActor* destroyedActor );

	// update in editor
	#if WITH_EDITOR
	virtual void PostEditChangeProperty( FPropertyChangedEvent& propertyChangedEvent ) override;
	#endif

private:
	float RuntimeWaveEndSafetyMargin_ = 1.0f;
};
