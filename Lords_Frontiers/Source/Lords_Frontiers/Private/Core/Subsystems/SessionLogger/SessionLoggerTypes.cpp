#include "Core/Subsystems/SessionLogger/SessionLoggerTypes.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"

// Helpers

namespace SessionLogHelper
{
	TSharedPtr<FJsonValue> IntPointToJson( const FIntPoint& Point )
	{
		TArray<TSharedPtr<FJsonValue>> Arr;
		Arr.Add( MakeShared<FJsonValueNumber>( Point.X ) );
		Arr.Add( MakeShared<FJsonValueNumber>( Point.Y ) );
		return MakeShared<FJsonValueArray>( Arr );
	}

	template <typename T>
	TSharedPtr<FJsonValue> ArrayToJson( const TArray<T>& Items )
	{
		TArray<TSharedPtr<FJsonValue>> Arr;
		Arr.Reserve( Items.Num() );
		for ( const T& Item : Items )
		{
			Arr.Add( MakeShared<FJsonValueObject>( Item.ToJson() ) );
		}
		return MakeShared<FJsonValueArray>( Arr );
	}

	TSharedPtr<FJsonValue> NameArrayToJson( const TArray<FName>& Names )
	{
		TArray<TSharedPtr<FJsonValue>> Arr;
		Arr.Reserve( Names.Num() );
		for ( const FName& Name : Names )
		{
			Arr.Add( MakeShared<FJsonValueString>( Name.ToString() ) );
		}
		return MakeShared<FJsonValueArray>( Arr );
	}
} // namespace SessionLogHelper

// FLogResourceSnapshot

TSharedPtr<FJsonObject> FLogResourceSnapshot::ToJson() const
{
	auto Obj = MakeShared<FJsonObject>();
	Obj->SetNumberField( TEXT( "gold" ), Gold );
	Obj->SetNumberField( TEXT( "food" ), Food );
	Obj->SetNumberField( TEXT( "population" ), Population );
	Obj->SetNumberField( TEXT( "progress" ), Progress );
	return Obj;
}

FLogResourceSnapshot FLogResourceSnapshot::operator-( const FLogResourceSnapshot& Other ) const
{
	FLogResourceSnapshot Result;
	Result.Gold = Gold - Other.Gold;
	Result.Food = Food - Other.Food;
	Result.Population = Population - Other.Population;
	Result.Progress = Progress - Other.Progress;
	return Result;
}

FLogResourceSnapshot FLogResourceSnapshot::operator+( const FLogResourceSnapshot& Other ) const
{
	FLogResourceSnapshot Result;
	Result.Gold = Gold + Other.Gold;
	Result.Food = Food + Other.Food;
	Result.Population = Population + Other.Population;
	Result.Progress = Progress + Other.Progress;
	return Result;
}

FLogResourceSnapshot& FLogResourceSnapshot::operator+=( const FLogResourceSnapshot& Other )
{
	Gold += Other.Gold;
	Food += Other.Food;
	Population += Other.Population;
	Progress += Other.Progress;
	return *this;
}

FLogResourceSnapshot FLogResourceSnapshot::FromProduction( const FResourceProduction& Prod )
{
	FLogResourceSnapshot Snap;
	Snap.Gold = Prod.Gold;
	Snap.Food = Prod.Food;
	Snap.Population = Prod.Population;
	Snap.Progress = Prod.Progress;
	return Snap;
}

// FLogBonusRecord

TSharedPtr<FJsonObject> FLogBonusRecord::ToJson() const
{
	auto Obj = MakeShared<FJsonObject>();
	Obj->SetStringField( TEXT( "sourceBuildingClass" ), SourceBuildingClass.ToString() );
	Obj->SetStringField( TEXT( "targetBuildingClass" ), TargetBuildingClass.ToString() );
	Obj->SetField( TEXT( "sourceCell" ), SessionLogHelper::IntPointToJson( SourceCell ) );
	Obj->SetField( TEXT( "targetCell" ), SessionLogHelper::IntPointToJson( TargetCell ) );
	Obj->SetStringField( TEXT( "category" ), Category );
	Obj->SetStringField( TEXT( "resourceOrStat" ), ResourceOrStat );
	Obj->SetNumberField( TEXT( "value" ), Value );
	return Obj;
}

// FLogBuildingPlacementRecord

TSharedPtr<FJsonObject> FLogBuildingPlacementRecord::ToJson() const
{
	auto Obj = MakeShared<FJsonObject>();
	Obj->SetStringField( TEXT( "className" ), BuildingClass.ToString() );
	Obj->SetStringField( TEXT( "category" ), Category );
	Obj->SetField( TEXT( "gridCoords" ), SessionLogHelper::IntPointToJson( CellCoords ) );
	Obj->SetObjectField( TEXT( "cost" ), Cost.ToJson() );
	Obj->SetField( TEXT( "bonusesReceived" ), SessionLogHelper::ArrayToJson( BonusesReceived ) );
	Obj->SetField( TEXT( "bonusesGiven" ), SessionLogHelper::ArrayToJson( BonusesGiven ) );
	return Obj;
}

// FLogBuildingCharacteristics

TSharedPtr<FJsonObject> FLogBuildingCharacteristics::ToJson() const
{
	auto Obj = MakeShared<FJsonObject>();
	Obj->SetStringField( TEXT( "className" ), BuildingClass.ToString() );
	Obj->SetStringField( TEXT( "category" ), Category );
	Obj->SetField( TEXT( "gridCoords" ), SessionLogHelper::IntPointToJson( CellCoords ) );
	Obj->SetNumberField( TEXT( "maxHealth" ), MaxHealth );
	if ( AttackDamage != 0 )
	{
		Obj->SetNumberField( TEXT( "attackDamage" ), AttackDamage );
	}
	if ( AttackRange > 0.0f )
	{
		Obj->SetNumberField( TEXT( "attackRange" ), AttackRange );
	}
	if ( AttackCooldown > 0.0f )
	{
		Obj->SetNumberField( TEXT( "attackCooldown" ), AttackCooldown );
	}
	if ( SplashRadius > 0.0f )
	{
		Obj->SetNumberField( TEXT( "splashRadius" ), SplashRadius );
	}
	if ( BurstCount > 1 )
	{
		Obj->SetNumberField( TEXT( "burstCount" ), BurstCount );
		Obj->SetNumberField( TEXT( "burstDelay" ), BurstDelay );
	}
	Obj->SetObjectField( TEXT( "maintenanceCost" ), MaintenanceCost.ToJson() );
	Obj->SetObjectField( TEXT( "buildingCost" ), BuildingCost.ToJson() );
	if ( Production.Gold != 0 || Production.Food != 0 || Production.Population != 0 || Production.Progress != 0 )
	{
		Obj->SetObjectField( TEXT( "production" ), Production.ToJson() );
	}
	return Obj;
}

// FLogBuildingDestroyedRecord

TSharedPtr<FJsonObject> FLogBuildingDestroyedRecord::ToJson() const
{
	auto Obj = MakeShared<FJsonObject>();
	Obj->SetStringField( TEXT( "className" ), BuildingClass.ToString() );
	Obj->SetStringField( TEXT( "category" ), Category );
	Obj->SetField( TEXT( "gridCoords" ), SessionLogHelper::IntPointToJson( CellCoords ) );
	return Obj;
}

// FLogTowerDamageRecord

TSharedPtr<FJsonObject> FLogTowerDamageRecord::ToJson() const
{
	auto Obj = MakeShared<FJsonObject>();
	Obj->SetStringField( TEXT( "towerClass" ), TowerClass.ToString() );
	Obj->SetStringField( TEXT( "attackType" ), AttackType );
	Obj->SetNumberField( TEXT( "directDamage" ), DirectDamage );
	Obj->SetNumberField( TEXT( "splashDamage" ), SplashDamage );
	Obj->SetNumberField( TEXT( "totalDamage" ), TotalDamage );
	Obj->SetNumberField( TEXT( "killCount" ), KillCount );
	Obj->SetNumberField( TEXT( "shotsTotal" ), ShotsTotal );
	Obj->SetNumberField( TEXT( "shotsHit" ), ShotsHit );
	return Obj;
}

// FLogBuildingDamageRecord

TSharedPtr<FJsonObject> FLogBuildingDamageRecord::ToJson() const
{
	auto Obj = MakeShared<FJsonObject>();
	Obj->SetStringField( TEXT( "className" ), BuildingClass.ToString() );
	Obj->SetField( TEXT( "gridCoords" ), SessionLogHelper::IntPointToJson( CellCoords ) );
	Obj->SetNumberField( TEXT( "healthBefore" ), HealthBefore );
	Obj->SetNumberField( TEXT( "healthAfter" ), HealthAfter );
	Obj->SetNumberField( TEXT( "damageReceived" ), DamageReceived );
	return Obj;
}

// FLogCombatSummary

TSharedPtr<FJsonObject> FLogCombatSummary::ToJson() const
{
	auto Obj = MakeShared<FJsonObject>();
	Obj->SetNumberField( TEXT( "mainBaseHealthStart" ), MainBaseHealthStart );
	Obj->SetNumberField( TEXT( "mainBaseHealthEnd" ), MainBaseHealthEnd );
	Obj->SetNumberField( TEXT( "mainBaseDamageReceived" ), MainBaseDamageReceived );
	Obj->SetNumberField( TEXT( "waveDurationSeconds" ), WaveDurationSeconds );
	Obj->SetField( TEXT( "buildingsDamaged" ), SessionLogHelper::ArrayToJson( BuildingsDamaged ) );
	return Obj;
}

// FLogDefensiveTurnData

TSharedPtr<FJsonObject> FLogDefensiveTurnData::ToJson() const
{
	auto Obj = MakeShared<FJsonObject>();
	Obj->SetObjectField( TEXT( "spentOnDefensive" ), SpentOnDefensive.ToJson() );
	Obj->SetObjectField( TEXT( "defensiveMaintenance" ), DefensiveMaintenance.ToJson() );
	Obj->SetField( TEXT( "defensivePlacements" ), SessionLogHelper::ArrayToJson( DefensivePlacements ) );
	Obj->SetField( TEXT( "adjacencyBonuses" ), SessionLogHelper::ArrayToJson( AdjacencyBonuses ) );
	Obj->SetField( TEXT( "allDefensiveStats" ), SessionLogHelper::ArrayToJson( AllDefensiveStats ) );
	Obj->SetField( TEXT( "towerDamage" ), SessionLogHelper::ArrayToJson( TowerDamage ) );
	if ( CombatSummary.MainBaseHealthStart > 0 )
	{
		Obj->SetObjectField( TEXT( "combatSummary" ), CombatSummary.ToJson() );
	}
	return Obj;
}

// FLogTurnData

TSharedPtr<FJsonObject> FLogTurnData::ToJson() const
{
	auto Obj = MakeShared<FJsonObject>();
	Obj->SetNumberField( TEXT( "turnNumber" ), TurnNumber );
	Obj->SetStringField( TEXT( "turnType" ), TurnType );
	Obj->SetNumberField( TEXT( "turnDurationSeconds" ), TurnDurationSeconds );

	// Economy sub-object
	auto EconObj = MakeShared<FJsonObject>();
	EconObj->SetObjectField( TEXT( "resourcesAtStart" ), ResourcesAtStart.ToJson() );
	EconObj->SetObjectField( TEXT( "incomeFromTransition" ), IncomeFromTransition.ToJson() );
	EconObj->SetObjectField( TEXT( "maintenancePaid" ), MaintenancePaid.ToJson() );
	EconObj->SetObjectField( TEXT( "spentOnBuilding" ), SpentOnBuilding.ToJson() );
	EconObj->SetObjectField( TEXT( "adjacencyBonusIncome" ), AdjacencyBonusIncome.ToJson() );
	EconObj->SetObjectField( TEXT( "cardProductionBonus" ), CardProductionBonus.ToJson() );
	EconObj->SetObjectField( TEXT( "cardMaintenanceReduction" ), CardMaintenanceReduction.ToJson() );
	EconObj->SetObjectField( TEXT( "resourcesAtEnd" ), ResourcesAtEnd.ToJson() );
	Obj->SetObjectField( TEXT( "economy" ), EconObj );

	Obj->SetField( TEXT( "buildingsPlaced" ), SessionLogHelper::ArrayToJson( BuildingsPlaced ) );
	Obj->SetField( TEXT( "buildingsDestroyed" ), SessionLogHelper::ArrayToJson( BuildingsDestroyed ) );
	Obj->SetField( TEXT( "allBuildingCharacteristics" ), SessionLogHelper::ArrayToJson( AllBuildingCharacteristics ) );
	Obj->SetObjectField( TEXT( "defensive" ), Defensive.ToJson() );
	return Obj;
}

// FLogEnemySpawnRecord

TSharedPtr<FJsonObject> FLogEnemySpawnRecord::ToJson() const
{
	auto Obj = MakeShared<FJsonObject>();
	Obj->SetStringField( TEXT( "className" ), EnemyClass.ToString() );
	Obj->SetNumberField( TEXT( "count" ), Count );
	Obj->SetStringField( TEXT( "spawnPointId" ), SpawnPointId.ToString() );
	Obj->SetNumberField( TEXT( "groupIndex" ), GroupIndex );
	return Obj;
}

// FLogEnemyTypeStats

TSharedPtr<FJsonObject> FLogEnemyTypeStats::ToJson() const
{
	auto Obj = MakeShared<FJsonObject>();
	Obj->SetStringField( TEXT( "className" ), EnemyClass.ToString() );
	Obj->SetNumberField( TEXT( "count" ), TotalCount );
	Obj->SetField( TEXT( "spawnPoints" ), SessionLogHelper::NameArrayToJson( SpawnPoints ) );

	// Stats sub-object (only non-zero)
	auto StatsObj = MakeShared<FJsonObject>();
	if ( MaxHealth > 0 )
	{
		StatsObj->SetNumberField( TEXT( "maxHealth" ), MaxHealth );
	}
	if ( AttackDamage > 0 )
	{
		StatsObj->SetNumberField( TEXT( "attackDamage" ), AttackDamage );
	}
	if ( AttackRange > 0.0f )
	{
		StatsObj->SetNumberField( TEXT( "attackRange" ), AttackRange );
	}
	if ( AttackCooldown > 0.0f )
	{
		StatsObj->SetNumberField( TEXT( "attackCooldown" ), AttackCooldown );
	}
	if ( MaxSpeed > 0.0f )
	{
		StatsObj->SetNumberField( TEXT( "maxSpeed" ), MaxSpeed );
	}
	if ( SplashRadius > 0.0f )
	{
		StatsObj->SetNumberField( TEXT( "splashRadius" ), SplashRadius );
	}
	if ( BurstCount > 1 )
	{
		StatsObj->SetNumberField( TEXT( "burstCount" ), BurstCount );
	}
	Obj->SetObjectField( TEXT( "stats" ), StatsObj );

	Obj->SetNumberField( TEXT( "killed" ), Killed );
	Obj->SetNumberField( TEXT( "reachedBase" ), ReachedBase );
	Obj->SetNumberField( TEXT( "avgPathProgress" ), AvgPathProgress );
	Obj->SetNumberField( TEXT( "maxPathProgress" ), MaxPathProgress );

	Obj->SetNumberField( TEXT( "damageToDefensive" ), DamageToDefensive );
	Obj->SetNumberField( TEXT( "damageToEconomic" ), DamageToEconomic );
	Obj->SetNumberField( TEXT( "damageToWalls" ), DamageToWalls );
	Obj->SetNumberField( TEXT( "totalDamage" ), TotalDamage );

	auto DestroyedObj = MakeShared<FJsonObject>();
	for ( const auto& Pair : BuildingsDestroyedCounts )
	{
		DestroyedObj->SetNumberField( Pair.Key.ToString(), Pair.Value );
	}
	Obj->SetObjectField( TEXT( "buildingsDestroyed" ), DestroyedObj );

	return Obj;
}

// FLogCardSelectionRecord

TSharedPtr<FJsonObject> FLogCardSelectionRecord::ToJson() const
{
	auto Obj = MakeShared<FJsonObject>();
	Obj->SetNumberField( TEXT( "waveNumber" ), WaveNumber );
	Obj->SetField( TEXT( "cardsOffered" ), SessionLogHelper::NameArrayToJson( CardsOffered ) );
	Obj->SetField( TEXT( "cardsTaken" ), SessionLogHelper::NameArrayToJson( CardsTaken ) );
	return Obj;
}

// FLogCellState

TSharedPtr<FJsonObject> FLogCellState::ToJson() const
{
	auto Obj = MakeShared<FJsonObject>();
	Obj->SetNumberField( TEXT( "x" ), Coords.X );
	Obj->SetNumberField( TEXT( "y" ), Coords.Y );
	Obj->SetBoolField( TEXT( "occupied" ), bIsOccupied );
	Obj->SetBoolField( TEXT( "buildable" ), bIsBuildable );
	Obj->SetBoolField( TEXT( "walkable" ), bIsWalkable );
	Obj->SetNumberField( TEXT( "buildBonus" ), BuildBonus );
	if ( bIsOccupied )
	{
		Obj->SetStringField( TEXT( "building" ), OccupantClass.ToString() );
		Obj->SetField( TEXT( "bonuses" ), SessionLogHelper::ArrayToJson( ActiveBonuses ) );
	}
	return Obj;
}

// FLogBuildMapSnapshot

TSharedPtr<FJsonObject> FLogBuildMapSnapshot::ToJson() const
{
	auto Obj = MakeShared<FJsonObject>();
	Obj->SetNumberField( TEXT( "waveNumber" ), WaveNumber );
	Obj->SetNumberField( TEXT( "width" ), Width );
	Obj->SetNumberField( TEXT( "height" ), Height );
	Obj->SetField( TEXT( "cells" ), SessionLogHelper::ArrayToJson( Cells ) );
	return Obj;
}

TSharedPtr<FJsonObject> FLogBuildMapSnapshot::ToDeltaJson( const FLogBuildMapSnapshot& Base ) const
{
	// Build lookup of base cells by coords
	TMap<FIntPoint, const FLogCellState*> BaseMap;
	for ( const FLogCellState& Cell : Base.Cells )
	{
		BaseMap.Add( Cell.Coords, &Cell );
	}

	// Collect only cells that differ from base
	TArray<TSharedPtr<FJsonValue>> ChangedArray;
	for ( const FLogCellState& Cell : Cells )
	{
		const FLogCellState** BaseCell = BaseMap.Find( Cell.Coords );
		bool bChanged = !BaseCell
		    || ( *BaseCell )->bIsOccupied != Cell.bIsOccupied
		    || ( *BaseCell )->bIsBuildable != Cell.bIsBuildable
		    || ( *BaseCell )->bIsWalkable != Cell.bIsWalkable
		    || !FMath::IsNearlyEqual( ( *BaseCell )->BuildBonus, Cell.BuildBonus )
		    || ( *BaseCell )->OccupantClass != Cell.OccupantClass;

		if ( bChanged )
		{
			ChangedArray.Add( MakeShared<FJsonValueObject>( Cell.ToJson() ) );
		}
	}

	auto Obj = MakeShared<FJsonObject>();
	Obj->SetNumberField( TEXT( "waveNumber" ), WaveNumber );
	Obj->SetNumberField( TEXT( "width" ), Width );
	Obj->SetNumberField( TEXT( "height" ), Height );
	Obj->SetBoolField( TEXT( "isDelta" ), true );
	Obj->SetField( TEXT( "cells" ), MakeShared<FJsonValueArray>( ChangedArray ) );
	return Obj;
}

// FLogWaveMetrics

TSharedPtr<FJsonObject> FLogWaveMetrics::ToJson() const
{
	auto Obj = MakeShared<FJsonObject>();
	Obj->SetNumberField( TEXT( "totalEnemyHP" ), TotalEnemyHP );
	Obj->SetNumberField( TEXT( "totalEnemyDPS" ), TotalEnemyDPS );
	Obj->SetNumberField( TEXT( "totalTowerDPS" ), TotalTowerDPS );
	Obj->SetNumberField( TEXT( "dpsToHpRatio" ), DpsToHpRatio );
	Obj->SetNumberField( TEXT( "enemyDiversityIndex" ), EnemyDiversityIndex );
	Obj->SetNumberField( TEXT( "difficultyScore" ), DifficultyScore );
	return Obj;
}

// FLogWaveData

TSharedPtr<FJsonObject> FLogWaveData::ToJson() const
{
	auto Obj = MakeShared<FJsonObject>();
	Obj->SetNumberField( TEXT( "waveNumber" ), WaveNumber );
	Obj->SetField( TEXT( "turns" ), SessionLogHelper::ArrayToJson( Turns ) );

	// Enemies
	auto EnemiesObj = MakeShared<FJsonObject>();
	EnemiesObj->SetField( TEXT( "spawned" ), SessionLogHelper::ArrayToJson( EnemiesSpawned ) );
	EnemiesObj->SetField( TEXT( "typeStats" ), SessionLogHelper::ArrayToJson( EnemyTypeStats ) );
	Obj->SetObjectField( TEXT( "enemies" ), EnemiesObj );

	Obj->SetObjectField( TEXT( "cards" ), CardSelection.ToJson() );
	Obj->SetObjectField( TEXT( "buildMap" ), BuildMap.ToJson() );
	Obj->SetObjectField( TEXT( "waveMetrics" ), WaveMetrics.ToJson() );
	return Obj;
}

TSharedPtr<FJsonObject> FLogWaveData::ToJson( const FLogBuildMapSnapshot& BaseMap ) const
{
	auto Obj = MakeShared<FJsonObject>();
	Obj->SetNumberField( TEXT( "waveNumber" ), WaveNumber );
	Obj->SetField( TEXT( "turns" ), SessionLogHelper::ArrayToJson( Turns ) );

	auto EnemiesObj = MakeShared<FJsonObject>();
	EnemiesObj->SetField( TEXT( "spawned" ), SessionLogHelper::ArrayToJson( EnemiesSpawned ) );
	EnemiesObj->SetField( TEXT( "typeStats" ), SessionLogHelper::ArrayToJson( EnemyTypeStats ) );
	Obj->SetObjectField( TEXT( "enemies" ), EnemiesObj );

	Obj->SetObjectField( TEXT( "cards" ), CardSelection.ToJson() );
	Obj->SetObjectField( TEXT( "buildMap" ), BuildMap.ToDeltaJson( BaseMap ) );
	Obj->SetObjectField( TEXT( "waveMetrics" ), WaveMetrics.ToJson() );
	return Obj;
}

// FLogSessionData

TSharedPtr<FJsonObject> FLogSessionData::ToJson() const
{
	auto RootObj = MakeShared<FJsonObject>();

	// Session info
	auto SessionObj = MakeShared<FJsonObject>();
	SessionObj->SetStringField( TEXT( "mapName" ), MapName );
	SessionObj->SetStringField( TEXT( "timestamp" ), Timestamp );
	SessionObj->SetStringField( TEXT( "outcome" ), bVictory ? TEXT( "Victory" ) : TEXT( "Defeat" ) );
	SessionObj->SetNumberField( TEXT( "wavesSurvived" ), WavesSurvived );
	SessionObj->SetObjectField( TEXT( "initialFieldState" ), InitialFieldState.ToJson() );
	SessionObj->SetObjectField( TEXT( "finalFieldState" ), FinalFieldState.ToJson() );

	// Cell popularity
	auto PopObj = MakeShared<FJsonObject>();
	for ( const auto& Pair : CellPopularity )
	{
		FString Key = FString::Printf( TEXT( "%d,%d" ), Pair.Key.X, Pair.Key.Y );
		PopObj->SetNumberField( Key, Pair.Value );
	}
	SessionObj->SetObjectField( TEXT( "cellPopularity" ), PopObj );

	// Economy totals
	SessionObj->SetObjectField( TEXT( "totalSpentOnBuilding" ), TotalSpentOnBuilding.ToJson() );
	SessionObj->SetObjectField( TEXT( "totalIncome" ), TotalIncome.ToJson() );
	SessionObj->SetObjectField( TEXT( "totalMaintenancePaid" ), TotalMaintenancePaid.ToJson() );

	// Card overall stats
	auto CardStatsObj = MakeShared<FJsonObject>();
	auto ShownObj = MakeShared<FJsonObject>();
	for ( const auto& Pair : CardsShownTotal )
	{
		ShownObj->SetNumberField( Pair.Key.ToString(), Pair.Value );
	}
	auto TakenObj = MakeShared<FJsonObject>();
	for ( const auto& Pair : CardsTakenTotal )
	{
		TakenObj->SetNumberField( Pair.Key.ToString(), Pair.Value );
	}
	CardStatsObj->SetObjectField( TEXT( "timesShown" ), ShownObj );
	CardStatsObj->SetObjectField( TEXT( "timesTaken" ), TakenObj );
	SessionObj->SetObjectField( TEXT( "cardOverallStats" ), CardStatsObj );

	// Session metrics
	auto MetricsObj = MakeShared<FJsonObject>();
	MetricsObj->SetNumberField( TEXT( "totalSessionDurationSeconds" ), TotalSessionDurationSeconds );
	MetricsObj->SetNumberField( TEXT( "totalBuildingsPlaced" ), TotalBuildingsPlaced );
	MetricsObj->SetNumberField( TEXT( "totalBuildingsLost" ), TotalBuildingsLost );
	MetricsObj->SetNumberField( TEXT( "totalEnemiesKilled" ), TotalEnemiesKilled );
	MetricsObj->SetNumberField( TEXT( "closestCallWave" ), ClosestCallWave );
	SessionObj->SetObjectField( TEXT( "sessionMetrics" ), MetricsObj );

	RootObj->SetObjectField( TEXT( "session" ), SessionObj );

	// Waves (use delta buildMap relative to initialFieldState)
	TArray<TSharedPtr<FJsonValue>> WaveArray;
	for ( const FLogWaveData& Wave : Waves )
	{
		WaveArray.Add( MakeShared<FJsonValueObject>( Wave.ToJson( InitialFieldState ) ) );
	}
	RootObj->SetField( TEXT( "waves" ), MakeShared<FJsonValueArray>( WaveArray ) );

	return RootObj;
}
