#pragma once

#include "Core/Subsystems/SessionLogger/SessionLoggerTypes.h"
#include "Core/GameLoopManager.h"

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "SessionLoggerSubsystem.generated.h"

class ISessionDataCollector;
class AWaveManager;
class UResourceManager;
class UEconomyComponent;
class AGridManager;
class ABuildManager;
class ABuilding;
class ADefensiveBuilding;
class AResourceBuilding;
class AUnit;
class AMainBase;
class UCardDataAsset;
class UBuildingBonusComponent;
struct FCardChoice;
struct FBonusApplication;
struct FBuildingBonusEntry;

DECLARE_MULTICAST_DELEGATE_OneParam( FOnWaveDataFinalized, const FLogWaveData& );
DECLARE_MULTICAST_DELEGATE_OneParam( FOnSessionFinalized, const FLogSessionData& );

/**
 * USessionLoggerSubsystem
 *
 * World subsystem that observes game events and collects session statistics.
 * Event-driven (no Tick). Writes compressed JSON (.json.gz) to Saved/SessionLogs/ at game end.
 *
 * Extensible via ISessionDataCollector interface and RegisterCollector().
 */
UCLASS()
class LORDS_FRONTIERS_API USessionLoggerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize( FSubsystemCollectionBase& Collection ) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem( UObject* Outer ) const override { return true; }

	// === Extensibility ===

	void RegisterCollector( ISessionDataCollector* Collector );
	void UnregisterCollector( ISessionDataCollector* Collector );

	const FLogSessionData& GetSessionData() const { return SessionData_; }
	int32 GetCurrentWaveNumber() const;
	bool IsLogging() const { return bIsLogging_; }

	FOnWaveDataFinalized OnWaveDataFinalized;
	FOnSessionFinalized OnSessionFinalized;

private:
	// === System Binding ===

	UFUNCTION()
	void OnCoreSystemsReady();

	void BindToSystems();
	void UnbindFromSystems();

	// === Delegate Handlers ===

	UFUNCTION()
	void HandlePhaseChanged( EGameLoopPhase OldPhase, EGameLoopPhase NewPhase );

	UFUNCTION()
	void HandleBuildTurnChanged( int32 CurrentTurn, int32 MaxTurns );

	UFUNCTION()
	void HandleWaveChanged( int32 CurrentWave, int32 TotalWaves );

	UFUNCTION()
	void HandleGameEnded( bool bVictory );

	UFUNCTION()
	void HandleWaveStarted( int32 WaveIndex );

	UFUNCTION()
	void HandleBuildingPlaced( ABuilding* Building, FIntPoint CellCoords );

	UFUNCTION()
	void HandleBuildingDied( ABuilding* Building );

	UFUNCTION()
	void HandleCardSelectionRequired( const FCardChoice& Choice );

	UFUNCTION()
	void HandleCardsApplied( const TArray<UCardDataAsset*>& AppliedCards );

	void HandleDamageDealt( AActor* Instigator, AActor* Target, int Damage, bool bIsSplash );

	// === Damage Sub-Handlers ===

	void AccumulateTowerDamage( ADefensiveBuilding* Tower, AActor* Target, int Damage, bool bIsSplash );
	void AccumulateEnemyDamage( AUnit* Enemy, AActor* Target, int Damage );

	// === Card Data Helpers ===

	FLogWaveData* GetWaveDataForCards( int32 WaveNumber );

	// === Bonus Helpers ===

	// Iterates validated bonus applications on a building, calling callback for each
	void ForEachBonusApplication(
		ABuilding* building,
		TFunctionRef<void( const FBonusApplication&, const FBuildingBonusEntry& )> callback
	) const;

	// Creates FLogBonusRecord from bonus entry data
	FLogBonusRecord MakeBonusRecord(
		const FBuildingBonusEntry& entry,
		FName targetClass,
		const FIntPoint& targetCell,
		float value
	) const;

	// === Data Capture Methods ===

	FLogResourceSnapshot CaptureCurrentResources() const;
	FLogBuildMapSnapshot CaptureBuildMapState( int32 waveNumber ) const;
	TArray<FLogBuildingCharacteristics> CaptureAllBuildingStats() const;
	TArray<FLogBuildingCharacteristics> CaptureDefensiveBuildingStats() const;
	void CaptureEnemySpawnData( int32 waveIndex );
	void CollectBonusDataForBuilding(
		ABuilding* building, const FIntPoint& buildingCoords,
		TArray<FLogBonusRecord>& OutReceived, TArray<FLogBonusRecord>& OutGiven
	);
	FString GetBuildingCategory( const ABuilding* building ) const;
	void CaptureBuildingHealthSnapshot();
	FLogCombatSummary BuildCombatSummary();
	FLogWaveMetrics CalculateWaveMetrics() const;
	void CalculateSessionMetrics();

	// === Turn Economy Sub-Methods ===

	void CaptureTurnEconomy();
	void CaptureDefensiveTurnData();
	void CaptureCombatTurnData();

	// === Wave/Turn Lifecycle ===

	void BeginNewWave( int32 waveNumber );
	void BeginNewTurn( int32 turnNumber, const FString& turnType );
	void EndCurrentTurn();
	void FinalizeWave();
	void FinalizeSession( bool bVictory );

	// === JSON Output ===

	void WriteSessionToFile();
	FString GetOutputFilePath() const;

	// === Index-Based Wave/Turn Access ===

	// Returns pointer to current wave data, or nullptr if no active wave
	FLogWaveData* GetCurrentWaveData();
	const FLogWaveData* GetCurrentWaveData() const;

	// Returns pointer to current turn data, or nullptr if no active turn
	FLogTurnData* GetCurrentTurnData();
	const FLogTurnData* GetCurrentTurnData() const;

	// === State ===

	FLogSessionData SessionData_;

	// Index-based references into SessionData_.Waves and Turns (safe across TArray reallocation)
	int32 CurrentWaveIndex_ = INDEX_NONE;
	int32 CurrentTurnIndex_ = INDEX_NONE;

	FLogResourceSnapshot ResourcesAtTurnStart_;
	FLogResourceSnapshot AccumulatedBuildCost_;
	FLogResourceSnapshot AccumulatedDefensiveBuildCost_;

	TArray<FName> LastOfferedCards_;

	// Damage accumulators (reset per wave)
	TMap<FName, FLogDamageAccumulator> TowerDamageMap_;
	TMap<FName, FLogEnemyDamageAccumulator> EnemyDamageMap_;

	// Building death tracking: building -> last enemy attacker
	TMap<TWeakObjectPtr<ABuilding>, FName> LastAttackerMap_;

	// Enemy kill credit tracking: enemy -> last tower class that hit it
	TMap<TWeakObjectPtr<AActor>, FName> EnemyLastTowerMap_;

	// Building HP snapshot at combat start (for damage tracking)
	TMap<TWeakObjectPtr<ABuilding>, int32> BuildingHealthAtCombatStart_;

	// Timing
	double SessionStartTime_ = 0.0;
	double TurnStartTime_ = 0.0;
	double CombatStartTime_ = 0.0;

	// Per-wave lowest base HP (for closest-call tracking)
	int32 LowestBaseHP_ = INT32_MAX;
	int32 ClosestCallWave_ = 0;

	// External data collectors
	TArray<ISessionDataCollector*> DataCollectors_;

	bool bIsLogging_ = false;
	bool bIsBound_ = false;
	int32 CurrentWaveNumber_ = 0;

	// Cached system references
	TWeakObjectPtr<UGameLoopManager> GameLoop_;
	TWeakObjectPtr<AWaveManager> WaveManager_;
	TWeakObjectPtr<UResourceManager> ResourceManager_;
	TWeakObjectPtr<UEconomyComponent> EconomyComponent_;
	TWeakObjectPtr<AGridManager> GridManager_;
	TWeakObjectPtr<ABuildManager> BuildManager_;

	FDelegateHandle DamageEventHandle_;
};
