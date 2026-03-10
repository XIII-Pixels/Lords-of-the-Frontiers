#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Resources/GameResource.h"

#include "SessionLoggerTypes.generated.h"

// ============================================================================
// Resource Snapshot
// ============================================================================

USTRUCT()
struct FLogResourceSnapshot
{
	GENERATED_BODY()

	int32 Gold = 0;
	int32 Food = 0;
	int32 Population = 0;
	int32 Progress = 0;

	TSharedPtr<FJsonObject> ToJson() const;

	FLogResourceSnapshot operator-( const FLogResourceSnapshot& Other ) const;
	FLogResourceSnapshot operator+( const FLogResourceSnapshot& Other ) const;
	FLogResourceSnapshot& operator+=( const FLogResourceSnapshot& Other );

	static FLogResourceSnapshot FromProduction( const FResourceProduction& Prod );
};

// ============================================================================
// Bonus Record
// ============================================================================

USTRUCT()
struct FLogBonusRecord
{
	GENERATED_BODY()

	FName SourceBuildingClass;
	FName TargetBuildingClass;
	FIntPoint SourceCell = FIntPoint( -1, -1 );
	FIntPoint TargetCell = FIntPoint( -1, -1 );
	FString Category; // Production / Maintenance / Stats
	FString ResourceOrStat;
	float Value = 0.0f;

	TSharedPtr<FJsonObject> ToJson() const;
};

// ============================================================================
// Building Placement Record
// ============================================================================

USTRUCT()
struct FLogBuildingPlacementRecord
{
	GENERATED_BODY()

	FName BuildingClass;
	FString Category; // Defensive / Resource / Additive / MainBase
	FIntPoint CellCoords = FIntPoint( -1, -1 );
	FLogResourceSnapshot Cost;
	TArray<FLogBonusRecord> BonusesReceived;
	TArray<FLogBonusRecord> BonusesGiven;

	TSharedPtr<FJsonObject> ToJson() const;
};

// ============================================================================
// Building Characteristics (without adjacency bonuses)
// ============================================================================

USTRUCT()
struct FLogBuildingCharacteristics
{
	GENERATED_BODY()

	FName BuildingClass;
	FString Category;
	FIntPoint CellCoords = FIntPoint( -1, -1 );
	int32 MaxHealth = 0;
	int32 AttackDamage = 0;
	float AttackRange = 0.0f;
	float AttackCooldown = 0.0f;
	float MaxSpeed = 0.0f;
	float SplashRadius = 0.0f;
	int32 BurstCount = 0;
	float BurstDelay = 0.0f;
	FLogResourceSnapshot MaintenanceCost;
	FLogResourceSnapshot BuildingCost;
	FLogResourceSnapshot Production; // only for resource buildings

	TSharedPtr<FJsonObject> ToJson() const;
};

// ============================================================================
// Building Destroyed Record
// ============================================================================

USTRUCT()
struct FLogBuildingDestroyedRecord
{
	GENERATED_BODY()

	FName BuildingClass;
	FString Category;
	FIntPoint CellCoords = FIntPoint( -1, -1 );

	TSharedPtr<FJsonObject> ToJson() const;
};

// ============================================================================
// Tower Damage Record
// ============================================================================

USTRUCT()
struct FLogTowerDamageRecord
{
	GENERATED_BODY()

	FName TowerClass;
	FString AttackType; // Ranged / Melee
	float DirectDamage = 0.0f;
	float SplashDamage = 0.0f;
	float TotalDamage = 0.0f;
	int32 KillCount = 0;
	int32 ShotsTotal = 0;
	int32 ShotsHit = 0;

	TSharedPtr<FJsonObject> ToJson() const;
};

// ============================================================================
// Building Damage Record (per-building damage during combat)
// ============================================================================

USTRUCT()
struct FLogBuildingDamageRecord
{
	GENERATED_BODY()

	FName BuildingClass;
	FIntPoint CellCoords = FIntPoint( -1, -1 );
	int32 HealthBefore = 0;
	int32 HealthAfter = 0;
	float DamageReceived = 0.0f;

	TSharedPtr<FJsonObject> ToJson() const;
};

// ============================================================================
// Combat Summary (aggregated combat results)
// ============================================================================

USTRUCT()
struct FLogCombatSummary
{
	GENERATED_BODY()

	int32 MainBaseHealthStart = 0;
	int32 MainBaseHealthEnd = 0;
	float MainBaseDamageReceived = 0.0f;
	float WaveDurationSeconds = 0.0f;
	TArray<FLogBuildingDamageRecord> BuildingsDamaged;

	TSharedPtr<FJsonObject> ToJson() const;
};

// ============================================================================
// Defensive Turn Data
// ============================================================================

USTRUCT()
struct FLogDefensiveTurnData
{
	GENERATED_BODY()

	FLogResourceSnapshot SpentOnDefensive;
	FLogResourceSnapshot DefensiveMaintenance;
	TArray<FLogBuildingPlacementRecord> DefensivePlacements;
	TArray<FLogBonusRecord> AdjacencyBonuses;
	TArray<FLogBuildingCharacteristics> AllDefensiveStats;
	TArray<FLogTowerDamageRecord> TowerDamage;
	FLogCombatSummary CombatSummary;

	TSharedPtr<FJsonObject> ToJson() const;
};

// ============================================================================
// Turn Data
// ============================================================================

USTRUCT()
struct FLogTurnData
{
	GENERATED_BODY()

	int32 TurnNumber = 0;
	FString TurnType; // Build / Combat

	// Economy
	FLogResourceSnapshot ResourcesAtStart;
	FLogResourceSnapshot IncomeFromTransition;
	FLogResourceSnapshot MaintenancePaid;
	FLogResourceSnapshot SpentOnBuilding;
	FLogResourceSnapshot AdjacencyBonusIncome;
	FLogResourceSnapshot CardProductionBonus;       // production bonus from cards
	FLogResourceSnapshot CardMaintenanceReduction;  // maintenance reduction from cards
	FLogResourceSnapshot ResourcesAtEnd;

	// Timing
	float TurnDurationSeconds = 0.0f;

	// Buildings
	TArray<FLogBuildingPlacementRecord> BuildingsPlaced;
	TArray<FLogBuildingDestroyedRecord> BuildingsDestroyed;
	TArray<FLogBuildingCharacteristics> AllBuildingCharacteristics;

	// Defense
	FLogDefensiveTurnData Defensive;

	TSharedPtr<FJsonObject> ToJson() const;
};

// ============================================================================
// Enemy Spawn Record
// ============================================================================

USTRUCT()
struct FLogEnemySpawnRecord
{
	GENERATED_BODY()

	FName EnemyClass;
	int32 Count = 0;
	FName SpawnPointId;
	int32 GroupIndex = 0;

	TSharedPtr<FJsonObject> ToJson() const;
};

// ============================================================================
// Enemy Type Stats
// ============================================================================

USTRUCT()
struct FLogEnemyTypeStats
{
	GENERATED_BODY()

	FName EnemyClass;

	// Spawn info (aggregated from spawn records)
	int32 TotalCount = 0;
	TArray<FName> SpawnPoints;

	// Non-zero stats
	int32 MaxHealth = 0;
	int32 AttackDamage = 0;
	float AttackRange = 0.0f;
	float AttackCooldown = 0.0f;
	float MaxSpeed = 0.0f;
	float SplashRadius = 0.0f;
	int32 BurstCount = 0;

	// Kill / penetration stats
	int32 Killed = 0;
	int32 ReachedBase = 0;
	float AvgPathProgress = 0.0f;
	float MaxPathProgress = 0.0f;

	// Damage dealt
	float DamageToDefensive = 0.0f;
	float DamageToEconomic = 0.0f;
	float DamageToWalls = 0.0f;
	float TotalDamage = 0.0f;

	// Buildings destroyed (class name -> count)
	TMap<FName, int32> BuildingsDestroyedCounts;

	TSharedPtr<FJsonObject> ToJson() const;
};

// ============================================================================
// Card Selection Record
// ============================================================================

USTRUCT()
struct FLogCardSelectionRecord
{
	GENERATED_BODY()

	int32 WaveNumber = 0;
	TArray<FName> CardsOffered;
	TArray<FName> CardsTaken;

	TSharedPtr<FJsonObject> ToJson() const;
};

// ============================================================================
// Cell State (for build map snapshot)
// ============================================================================

USTRUCT()
struct FLogCellState
{
	GENERATED_BODY()

	FIntPoint Coords = FIntPoint( -1, -1 );
	bool bIsOccupied = false;
	bool bIsBuildable = false;
	bool bIsWalkable = false;
	float BuildBonus = 0.0f;
	FName OccupantClass; // NAME_None if empty
	TArray<FLogBonusRecord> ActiveBonuses;

	TSharedPtr<FJsonObject> ToJson() const;
};

// ============================================================================
// Build Map Snapshot
// ============================================================================

USTRUCT()
struct FLogBuildMapSnapshot
{
	GENERATED_BODY()

	int32 WaveNumber = 0;
	int32 Width = 0;
	int32 Height = 0;
	TArray<FLogCellState> Cells;

	TSharedPtr<FJsonObject> ToJson() const;

	// Delta serialization: only cells that differ from Base
	TSharedPtr<FJsonObject> ToDeltaJson( const FLogBuildMapSnapshot& Base ) const;
};

// ============================================================================
// Wave Metrics (calculated balance analytics)
// ============================================================================

USTRUCT()
struct FLogWaveMetrics
{
	GENERATED_BODY()

	int32 TotalEnemyHP = 0;
	float TotalEnemyDPS = 0.0f;
	float TotalTowerDPS = 0.0f;
	float DpsToHpRatio = 0.0f;
	int32 EnemyDiversityIndex = 0;
	float DifficultyScore = 0.0f;

	TSharedPtr<FJsonObject> ToJson() const;
};

// ============================================================================
// Wave Data (aggregation of all data for one wave)
// ============================================================================

USTRUCT()
struct FLogWaveData
{
	GENERATED_BODY()

	int32 WaveNumber = 0;

	// Per-turn data
	TArray<FLogTurnData> Turns;

	// Enemies
	TArray<FLogEnemySpawnRecord> EnemiesSpawned;
	TArray<FLogEnemyTypeStats> EnemyTypeStats;

	// Cards
	FLogCardSelectionRecord CardSelection;

	// Build map snapshot at end of wave
	FLogBuildMapSnapshot BuildMap;

	// Wave analytics
	FLogWaveMetrics WaveMetrics;

	TSharedPtr<FJsonObject> ToJson() const;
	TSharedPtr<FJsonObject> ToJson( const FLogBuildMapSnapshot& BaseMap ) const;
};

// ============================================================================
// Session Data (top-level)
// ============================================================================

USTRUCT()
struct FLogSessionData
{
	GENERATED_BODY()

	FString MapName;
	FString Timestamp;
	bool bVictory = false;
	int32 WavesSurvived = 0;

	// Initial and final field state
	FLogBuildMapSnapshot InitialFieldState;
	FLogBuildMapSnapshot FinalFieldState;

	// Cell popularity: how many times each cell had a building
	TMap<FIntPoint, int32> CellPopularity;

	// Economy totals
	FLogResourceSnapshot TotalSpentOnBuilding;
	FLogResourceSnapshot TotalIncome;
	FLogResourceSnapshot TotalMaintenancePaid;

	// Card overall stats
	TMap<FName, int32> CardsShownTotal;
	TMap<FName, int32> CardsTakenTotal;

	// Session-level analytics
	float TotalSessionDurationSeconds = 0.0f;
	int32 TotalBuildingsPlaced = 0;
	int32 TotalBuildingsLost = 0;
	int32 TotalEnemiesKilled = 0;
	int32 ClosestCallWave = 0; // Wave where base HP was lowest

	// Per-wave data
	TArray<FLogWaveData> Waves;

	TSharedPtr<FJsonObject> ToJson() const;
};

// ============================================================================
// Damage Accumulator (internal helper for combat damage tracking)
// ============================================================================

struct FLogDamageAccumulator
{
	FName InstigatorClass;
	FString AttackType; // Ranged / Melee
	float DirectDamage = 0.0f;
	float SplashDamage = 0.0f;
	int32 KillCount = 0;
	int32 ShotsTotal = 0;
	int32 ShotsHit = 0;
};

struct FLogEnemyDamageAccumulator
{
	FName EnemyClass;
	float DamageToDefensive = 0.0f;
	float DamageToEconomic = 0.0f;
	float DamageToWalls = 0.0f;
	float TotalDamage = 0.0f;
	TMap<FName, int32> BuildingsDestroyedCounts;

	// Penetration tracking
	int32 Killed = 0;
	int32 ReachedBase = 0;
	TArray<float> PathProgressValues; // Collected per-unit for averaging
};
