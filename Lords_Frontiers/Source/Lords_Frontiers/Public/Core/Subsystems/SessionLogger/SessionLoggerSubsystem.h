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
struct FCardChoice;

DECLARE_MULTICAST_DELEGATE_OneParam( FOnWaveDataFinalized, const FLogWaveData& );
DECLARE_MULTICAST_DELEGATE_OneParam( FOnSessionFinalized, const FLogSessionData& );

/**
 * USessionLoggerSubsystem
 *
 * World subsystem that observes game events and collects session statistics.
 * Event-driven (no Tick). Writes JSON to Saved/SessionLogs/ at game end.
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

	void HandleDamageDealt( AActor* Instigator, AActor* Target, float Damage, bool bIsSplash );

	// === Data Capture Methods ===

	FLogResourceSnapshot CaptureCurrentResources() const;
	FLogBuildMapSnapshot CaptureBuildMapState( int32 WaveNumber ) const;
	TArray<FLogBuildingCharacteristics> CaptureAllBuildingStats() const;
	TArray<FLogBuildingCharacteristics> CaptureDefensiveBuildingStats() const;
	void CaptureEnemySpawnData( int32 WaveIndex );
	void CollectBonusDataForBuilding(
	    ABuilding* Bldg, const FIntPoint& BldgCoords, TArray<FLogBonusRecord>& OutReceived,
	    TArray<FLogBonusRecord>& OutGiven
	);
	FString GetBuildingCategory( const ABuilding* Building ) const;
	void CaptureBuildingHealthSnapshot();
	FLogCombatSummary BuildCombatSummary();
	FLogWaveMetrics CalculateWaveMetrics() const;
	void CalculateSessionMetrics();

	// === Wave/Turn Lifecycle ===

	void BeginNewWave( int32 WaveNumber );
	void BeginNewTurn( int32 TurnNumber, const FString& TurnType );
	void EndCurrentTurn();
	void FinalizeWave();
	void FinalizeSession( bool bVictory );

	// === JSON Output ===

	void WriteSessionToFile();
	FString GetOutputFilePath() const;

	// === State ===

	FLogSessionData SessionData_;
	FLogWaveData* CurrentWaveData_ = nullptr;
	FLogTurnData* CurrentTurnData_ = nullptr;

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
