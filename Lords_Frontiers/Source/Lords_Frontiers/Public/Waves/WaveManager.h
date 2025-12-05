#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Wave.h"
#include "EnemyGroup.h"
#include "EnemyGroupSpawnPoint.h"
#include "Lords_Frontiers/Public/Unit.h"
#include "WaveManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveStartedSignature, int32, WaveIndex); //needed to call from BP
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveEndedSignature, int32, WaveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllWavesCompletedSignature);

/*
 WaveManager - actor placed on level to manage sequencing of waves
 (Artyom)
 */
UCLASS(Blueprintable)
class LORDS_FRONTIERS_API AWaveManager : public AActor
{
	GENERATED_BODY ()

public:
	AWaveManager ();

	// Start playing waves from CurrentWaveIndex (or first wave if CurrentWaveIndex is invalid).
	UFUNCTION ( BlueprintCallable, Category = "Wave" )
	void StartWaves();

	// Start wave by index (0-based). 
	UFUNCTION ( BlueprintCallable, Category = "Wave" )
	void StartWaveAtIndex ( int32 waveIndex );

	// Advance to next wave manually (skips waiting). If there are no more waves, broadcasts completion
	UFUNCTION ( BlueprintCallable, Category = "Wave" )
	void AdvanceToNextWave ();

	// Stop/cancel current wave (clears timers). Useful for editor or reset
	UFUNCTION ( BlueprintCallable, Category = "Wave" )
	void CancelCurrentWave ();

	// Restart waves from the beginning (index 0)
	UFUNCTION ( BlueprintCallable, Category = "Wave" )
	void RestartWaves ();

	//Returns whether a wave is currently active (spawning)
	UFUNCTION ( BlueprintPure, Category = "Wave" )
	bool IsWaveActive () const { return bIsWaveActive; }

	UFUNCTION(BlueprintPure, Category = "Wave")
	bool IsFirstWaveRequested () const { return bHasRequestedFirstWave; }

	// Current wave index (0-based)
	UPROPERTY ( EditAnywhere, BlueprintReadWrite, Category = "Wave" )
	int32 CurrentWaveIndex = 0;

	// List of waves. Index in array is the wave number
	UPROPERTY ( EditAnywhere, BlueprintReadWrite, Category = "Wave" )
	TArray<FWave> Waves;

	// If true, WaveManager will auto-start the first wave on BeginPlay (if there are waves)
	UPROPERTY ( EditAnywhere, BlueprintReadWrite, Category = "Wave" )
	bool bAutoStartOnBeginPlay = false;

	// If true, logs debug messages about spawning
	UPROPERTY ( EditAnywhere, BlueprintReadWrite, Category = "Wave" )
	bool bLogSpawning = true;

	// Delegate: broadcast when a wave starts
	UPROPERTY ( BlueprintAssignable, Category = "Wave|Events" )
	FOnWaveStartedSignature OnWaveStarted;

	// Delegate: broadcast when a wave ends (all groups finished)
	UPROPERTY ( BlueprintAssignable, Category = "Wave|Events" )
	FOnWaveEndedSignature OnWaveEnded;

	// Delegate: broadcast when all waves in Waves[] have been completed
	UPROPERTY ( BlueprintAssignable, Category = "Wave|Events" )
	FOnAllWavesCompletedSignature OnAllWavesCompleted;

protected:
	virtual void BeginPlay () override;

	//Internal helper to schedule all spawn timers for a wave
	void ScheduleWaveSpawns ( const FWave& wave, int32 waveIndex );

	//Schedules spawn for a single enemy (called via timer)
	UFUNCTION ()
	void SpawnEnemy ( int32 waveIndex, int32 groupIndex, int32 enemyIndex );

	// Called when a scheduled wave-end timer elapses
	UFUNCTION ()
	void OnWaveEndTimerElapsed ( int32 waveIndex );

	// Clean up timers related to the current wave
	void ClearActiveTimers ();

	// Move to next wave index and optionally start it. Returns true if there was a next wave
	bool MoveToNextWaveAndStart ();

	// Find spawn point near Enemygroupspawnpoint in case unit is already in it
	FTransform FindNonOverlappingSpawnTransform ( const FTransform& DesiredTransform, float CapsuleRadius, float CapsuleHalfHeight, float MaxSearchRadius = 600.f,
		int32 MaxAttempts = 32, bool bProjectToNavMesh = false ) const;

protected:
	// Active timer handles (for enemy spawns) so we can clear them if needed
	UPROPERTY ( Transient )
	TArray <FTimerHandle> ActiveSpawnTimers;

	// Active timer handle for wave end 
	UPROPERTY ( Transient )
	FTimerHandle WaveEndTimerHandle;

	// True while a wave is active (spawning or waiting for last spawn to finish)
	UPROPERTY ( VisibleAnywhere, BlueprintReadOnly, Category = "Wave" )
	bool bIsWaveActive = false;

	UPROPERTY(Transient)
	bool bHasRequestedFirstWave = false;

	// Utility: returns wave index clamped into array bounds (or INDEX_NONE if no waves)
	int32 ClampWaveIndex ( int32 waveIndex ) const;
};