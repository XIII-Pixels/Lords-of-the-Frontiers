#include "Core/Subsystems/SessionLogger/SessionLoggerSubsystem.h"

#include "Async/Async.h"
#include "Building/AdditiveBuilding.h"
#include "Building/Bonus/BuildingBonusComponent.h"
#include "Building/Building.h"
#include "Building/Construction/BuildManager.h"
#include "Building/DefensiveBuilding.h"
#include "Building/MainBase.h"
#include "Building/ResourceBuilding.h"
#include "Cards/CardDataAsset.h"
#include "Cards/CardSubsystem.h"
#include "Core/CoreManager.h"
#include "Core/GameLoopManager.h"
#include "Core/Subsystems/SessionLogger/DamageEvent.h"
#include "Core/Subsystems/SessionLogger/ISessionDataCollector.h"
#include "Dom/JsonObject.h"
#include "Grid/GridCell.h"
#include "Grid/GridManager.h"
#include "Resources/EconomyComponent.h"
#include "Resources/ResourceManager.h"
#include "Units/Unit.h"
#include "Waves/EnemyBuff.h"
#include "Waves/EnemyGroup.h"
#include "Waves/Wave.h"
#include "Waves/WaveManager.h"

#include "EngineUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

#include <zlib.h>

DEFINE_LOG_CATEGORY_STATIC( LogSessionLogger, Log, All );

// Named constants
static constexpr int32 cExpectedMaxWaves = 20;
static constexpr int32 cExpectedTurnsPerWave = 3;
static constexpr int32 cCombatTurnNumber = 3;
static constexpr float cPathProgressDistanceScale = 1.5f;
static constexpr float cCellSizeCm = 100.0f;
static constexpr float cDifficultyScoreMultiplier = 5.0f;

// Index-Based Wave/Turn Accessors

FLogWaveData* USessionLoggerSubsystem::GetCurrentWaveData()
{
	if ( CurrentWaveIndex_ != INDEX_NONE && SessionData_.Waves.IsValidIndex( CurrentWaveIndex_ ) )
	{
		return &SessionData_.Waves[CurrentWaveIndex_];
	}
	return nullptr;
}

const FLogWaveData* USessionLoggerSubsystem::GetCurrentWaveData() const
{
	if ( CurrentWaveIndex_ != INDEX_NONE && SessionData_.Waves.IsValidIndex( CurrentWaveIndex_ ) )
	{
		return &SessionData_.Waves[CurrentWaveIndex_];
	}
	return nullptr;
}

FLogTurnData* USessionLoggerSubsystem::GetCurrentTurnData()
{
	FLogWaveData* waveData = GetCurrentWaveData();
	if ( waveData && CurrentTurnIndex_ != INDEX_NONE && waveData->Turns.IsValidIndex( CurrentTurnIndex_ ) )
	{
		return &waveData->Turns[CurrentTurnIndex_];
	}
	return nullptr;
}

const FLogTurnData* USessionLoggerSubsystem::GetCurrentTurnData() const
{
	const FLogWaveData* waveData = GetCurrentWaveData();
	if ( waveData && CurrentTurnIndex_ != INDEX_NONE && waveData->Turns.IsValidIndex( CurrentTurnIndex_ ) )
	{
		return &waveData->Turns[CurrentTurnIndex_];
	}
	return nullptr;
}

// Lifecycle

void USessionLoggerSubsystem::Initialize( FSubsystemCollectionBase& Collection )
{
	Super::Initialize( Collection );

	SessionData_.Waves.Reserve( cExpectedMaxWaves );

	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		core->OnSystemsReady.AddDynamic( this, &USessionLoggerSubsystem::OnCoreSystemsReady );

		if ( core->AreCriticalSystemsReady() )
		{
			OnCoreSystemsReady();
		}
	}
}

void USessionLoggerSubsystem::OnWorldBeginPlay( UWorld& InWorld )
{
	Super::OnWorldBeginPlay( InWorld );

	if ( !bIsBound_ )
	{
		if ( UCoreManager* core = UCoreManager::Get( this ) )
		{
			if ( !core->OnSystemsReady.IsAlreadyBound( this, &USessionLoggerSubsystem::OnCoreSystemsReady ) )
			{
				core->OnSystemsReady.AddDynamic( this, &USessionLoggerSubsystem::OnCoreSystemsReady );
			}

			if ( core->AreCriticalSystemsReady() )
			{
				OnCoreSystemsReady();
			}
		}
	}
}

void USessionLoggerSubsystem::OnCoreSystemsReady()
{
	BindToSystems();
}

void USessionLoggerSubsystem::Deinitialize()
{
	// If the world is torn down while a session is active (e.g. level reload),
	// write whatever data we have. Skip full finalization — actors are already
	// destroyed at this point, so FinalizeWave/EndCurrentTurn would crash.
	if ( bIsLogging_ )
	{
		SessionData_.bVictory = false;
		SessionData_.WavesSurvived = CurrentWaveNumber_;
		SessionOutcome_ = TEXT( "RestartExit" );
		SessionData_.TotalSessionDurationSeconds = static_cast<float>( FPlatformTime::Seconds() - SessionStartTime_ );

		WriteSessionToFile();
		bIsLogging_ = false;
	}

	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		core->OnSystemsReady.RemoveDynamic( this, &USessionLoggerSubsystem::OnCoreSystemsReady );
	}

	UnbindFromSystems();
	Super::Deinitialize();
}

// System Binding

void USessionLoggerSubsystem::BindToSystems()
{
	if ( bIsBound_ )
	{
		return;
	}

	UCoreManager* core = UCoreManager::Get( this );
	if ( !core )
	{
		return;
	}

	GameLoop_ = core->GetGameLoop();
	WaveManager_ = core->GetWaveManager();
	ResourceManager_ = core->GetResourceManager();
	EconomyComponent_ = core->GetEconomyComponent();
	GridManager_ = core->GetGridManager();
	BuildManager_ = core->GetBuildManager();

	if ( GameLoop_.IsValid() )
	{
		GameLoop_->OnPhaseChanged.AddUniqueDynamic( this, &USessionLoggerSubsystem::HandlePhaseChanged );
		GameLoop_->OnBuildTurnChanged.AddUniqueDynamic( this, &USessionLoggerSubsystem::HandleBuildTurnChanged );
		GameLoop_->OnWaveChanged.AddUniqueDynamic( this, &USessionLoggerSubsystem::HandleWaveChanged );
		GameLoop_->OnGameEnded.AddUniqueDynamic( this, &USessionLoggerSubsystem::HandleGameEnded );
	}

	if ( WaveManager_.IsValid() )
	{
		WaveManager_->OnWaveStarted.AddUniqueDynamic( this, &USessionLoggerSubsystem::HandleWaveStarted );
	}

	if ( BuildManager_.IsValid() )
	{
		BuildManager_->OnBuildingConfirmed.AddUniqueDynamic( this, &USessionLoggerSubsystem::HandleBuildingPlaced );
	}

	if ( UCardSubsystem* cardSub = UCardSubsystem::Get( this ) )
	{
		cardSub->OnCardSelectionRequired.AddUniqueDynamic(
		    this, &USessionLoggerSubsystem::HandleCardSelectionRequired
		);
		cardSub->OnCardsApplied.AddUniqueDynamic( this, &USessionLoggerSubsystem::HandleCardsApplied );
		UE_LOG( LogSessionLogger, Log, TEXT( "Bound to CardSubsystem events" ) );
	}
	else
	{
		UE_LOG(
		    LogSessionLogger, Warning,
		    TEXT( "CardSubsystem not found during BindToSystems — card events will not be logged!" )
		);
	}

	DamageEventHandle_ = FDamageEvents::OnDamageDealt.AddUObject( this, &USessionLoggerSubsystem::HandleDamageDealt );

	bIsBound_ = true;

	UE_LOG( LogSessionLogger, Log, TEXT( "SessionLogger bound to all systems" ) );
}

void USessionLoggerSubsystem::UnbindFromSystems()
{
	if ( !bIsBound_ )
	{
		return;
	}

	if ( GameLoop_.IsValid() )
	{
		GameLoop_->OnPhaseChanged.RemoveDynamic( this, &USessionLoggerSubsystem::HandlePhaseChanged );
		GameLoop_->OnBuildTurnChanged.RemoveDynamic( this, &USessionLoggerSubsystem::HandleBuildTurnChanged );
		GameLoop_->OnWaveChanged.RemoveDynamic( this, &USessionLoggerSubsystem::HandleWaveChanged );
		GameLoop_->OnGameEnded.RemoveDynamic( this, &USessionLoggerSubsystem::HandleGameEnded );
	}

	if ( WaveManager_.IsValid() )
	{
		WaveManager_->OnWaveStarted.RemoveDynamic( this, &USessionLoggerSubsystem::HandleWaveStarted );
	}

	if ( BuildManager_.IsValid() )
	{
		BuildManager_->OnBuildingConfirmed.RemoveDynamic( this, &USessionLoggerSubsystem::HandleBuildingPlaced );
	}

	if ( UCardSubsystem* cardSub = UCardSubsystem::Get( this ) )
	{
		cardSub->OnCardSelectionRequired.RemoveDynamic( this, &USessionLoggerSubsystem::HandleCardSelectionRequired );
		cardSub->OnCardsApplied.RemoveDynamic( this, &USessionLoggerSubsystem::HandleCardsApplied );
	}

	FDamageEvents::OnDamageDealt.Remove( DamageEventHandle_ );

	bIsBound_ = false;
}

// Extensibility

void USessionLoggerSubsystem::RegisterCollector( ISessionDataCollector* collector )
{
	if ( collector )
	{
		DataCollectors_.AddUnique( collector );
	}
}

void USessionLoggerSubsystem::UnregisterCollector( ISessionDataCollector* collector )
{
	DataCollectors_.Remove( collector );
}

int32 USessionLoggerSubsystem::GetCurrentWaveNumber() const
{
	return CurrentWaveNumber_;
}

// Phase Changed Handler

void USessionLoggerSubsystem::HandlePhaseChanged( EGameLoopPhase oldPhase, EGameLoopPhase newPhase )
{
	for ( ISessionDataCollector* collector : DataCollectors_ )
	{
		if ( collector )
		{
			collector->OnPhaseChanged( oldPhase, newPhase );
		}
	}

	switch ( newPhase )
	{
	case EGameLoopPhase::Building:
	{
		if ( oldPhase == EGameLoopPhase::Startup || oldPhase == EGameLoopPhase::None )
		{
			bIsLogging_ = true;
			SessionStartTime_ = FPlatformTime::Seconds();
			LowestBaseHP_ = INT32_MAX;
			ClosestCallWave_ = 0;
			SessionData_ = FLogSessionData();

			UWorld* world = GetWorld();
			if ( world )
			{
				SessionData_.MapName = world->GetMapName();
				SessionData_.MapName.RemoveFromStart( world->StreamingLevelsPrefix );
			}
			SessionData_.Timestamp = FDateTime::Now().ToString( TEXT( "%Y-%m-%dT%H:%M:%S" ) );

			SessionData_.InitialFieldState = CaptureBuildMapState( 0 );

			CurrentWaveNumber_ = 1;
			BeginNewWave( CurrentWaveNumber_ );
			BeginNewTurn( 1, TEXT( "Build" ) );
		}
		else if ( oldPhase == EGameLoopPhase::Reward )
		{
			BeginNewWave( CurrentWaveNumber_ );
			BeginNewTurn( 1, TEXT( "Build" ) );
		}
		break;
	}
	case EGameLoopPhase::Combat:
	{
		EndCurrentTurn();
		BeginNewTurn( cCombatTurnNumber, TEXT( "Combat" ) );
		CombatStartTime_ = FPlatformTime::Seconds();

		TowerDamageMap_.Reset();
		EnemyDamageMap_.Reset();
		LastAttackerMap_.Reset();
		EnemyLastTowerMap_.Reset();

		CaptureBuildingHealthSnapshot();

		UWorld* world = GetWorld();
		if ( world )
		{
			for ( TActorIterator<ABuilding> it( world ); it; ++it )
			{
				ABuilding* bldg = *it;
				if ( bldg && !bldg->IsRuined() )
				{
					bldg->OnBuildingDied.AddUniqueDynamic( this, &USessionLoggerSubsystem::HandleBuildingDied );
				}
			}
		}

		break;
	}
	case EGameLoopPhase::Reward:
	{
		EndCurrentTurn();
		FinalizeWave();
		break;
	}
	case EGameLoopPhase::Victory:
	{
		FinalizeSession( true );
		break;
	}
	case EGameLoopPhase::Defeat:
	{
		FinalizeSession( false );
		break;
	}
	default:
		break;
	}
}

// Turn Changed Handler

void USessionLoggerSubsystem::HandleBuildTurnChanged( int32 currentTurn, int32 maxTurns )
{
	for ( ISessionDataCollector* collector : DataCollectors_ )
	{
		if ( collector )
		{
			collector->OnTurnChanged( currentTurn, maxTurns );
		}
	}

	EndCurrentTurn();
	BeginNewTurn( currentTurn, TEXT( "Build" ) );
}

// Wave Changed Handler

void USessionLoggerSubsystem::HandleWaveChanged( int32 currentWave, int32 totalWaves )
{
	CurrentWaveNumber_ = currentWave;

	for ( ISessionDataCollector* collector : DataCollectors_ )
	{
		if ( collector )
		{
			collector->OnWaveChanged( currentWave, totalWaves );
		}
	}
}

// Game Ended Handler

void USessionLoggerSubsystem::HandleGameEnded( bool bVictory )
{
	// No-op: FinalizeSession handles collector notification via HandlePhaseChanged (Victory/Defeat)
}

// Wave Started Handler

void USessionLoggerSubsystem::HandleWaveStarted( int32 waveIndex )
{
	CaptureEnemySpawnData( waveIndex );
}

// Building Placed Handler

void USessionLoggerSubsystem::HandleBuildingPlaced( ABuilding* building, FIntPoint cellCoords )
{
	FLogTurnData* turnData = GetCurrentTurnData();
	if ( !turnData || !building )
	{
		return;
	}

	FLogBuildingPlacementRecord record;
	record.BuildingClass = building->GetClass()->GetFName();
	record.Category = GetBuildingCategory( building );
	record.CellCoords = cellCoords;
	record.Cost = FLogResourceSnapshot::FromProduction( building->GetBuildingCost() );

	CollectBonusDataForBuilding( building, cellCoords, record.BonusesReceived, record.BonusesGiven );

	turnData->BuildingsPlaced.Add( record );

	AccumulatedBuildCost_ += record.Cost;
	SessionData_.TotalBuildingsPlaced++;

	int32& count = SessionData_.CellPopularity.FindOrAdd( cellCoords );
	count++;

	if ( Cast<ADefensiveBuilding>( building ) )
	{
		AccumulatedDefensiveBuildCost_ += record.Cost;
		turnData->Defensive.DefensivePlacements.Add( record );
	}
}

// Building Died Handler

void USessionLoggerSubsystem::HandleBuildingDied( ABuilding* building )
{
	FLogTurnData* turnData = GetCurrentTurnData();
	if ( !turnData || !building )
	{
		return;
	}

	FLogBuildingDestroyedRecord record;
	record.BuildingClass = building->GetClass()->GetFName();
	record.Category = GetBuildingCategory( building );

	if ( GridManager_.IsValid() )
	{
		record.CellCoords = GridManager_->GetClosestCellCoords( building->GetActorLocation() );
	}

	turnData->BuildingsDestroyed.Add( record );
	SessionData_.TotalBuildingsLost++;

	FName* lastAttacker = LastAttackerMap_.Find( building );
	if ( lastAttacker )
	{
		FLogEnemyDamageAccumulator& enemyAcc = EnemyDamageMap_.FindOrAdd( *lastAttacker );
		enemyAcc.EnemyClass = *lastAttacker;
		enemyAcc.BuildingsDestroyedCounts.FindOrAdd( record.BuildingClass )++;
	}

	building->OnBuildingDied.RemoveDynamic( this, &USessionLoggerSubsystem::HandleBuildingDied );
}

// Card Helpers

FLogWaveData* USessionLoggerSubsystem::GetWaveDataForCards( int32 waveNumber )
{
	FLogWaveData* waveData = GetCurrentWaveData();
	if ( waveData )
	{
		return waveData;
	}

	UE_LOG(
	    LogSessionLogger, Warning, TEXT( "CurrentWaveData is null during card event (wave %d), searching by number" ),
	    waveNumber
	);

	for ( FLogWaveData& wave : SessionData_.Waves )
	{
		if ( wave.WaveNumber == waveNumber )
		{
			return &wave;
		}
	}

	UE_LOG(
	    LogSessionLogger, Error, TEXT( "Could not find wave %d in SessionData — card data will be lost!" ), waveNumber
	);
	return nullptr;
}

// Card Handlers

void USessionLoggerSubsystem::HandleCardSelectionRequired( const FCardChoice& choice )
{
	LastOfferedCards_.Reset();
	for ( const auto& card : choice.AvailableCards )
	{
		if ( card )
		{
			LastOfferedCards_.Add( card->CardID );
			int32& shownCount = SessionData_.CardsShownTotal.FindOrAdd( card->CardID );
			shownCount++;
		}
	}

	FLogWaveData* waveData = GetWaveDataForCards( choice.WaveNumber );
	if ( waveData )
	{
		waveData->CardSelection.WaveNumber = choice.WaveNumber;
		waveData->CardSelection.CardsOffered = LastOfferedCards_;
	}
}

void USessionLoggerSubsystem::HandleCardsApplied( const TArray<UCardDataAsset*>& appliedCards )
{
	TArray<FName> takenIds;
	for ( const UCardDataAsset* card : appliedCards )
	{
		if ( card )
		{
			takenIds.Add( card->CardID );
			int32& takenCount = SessionData_.CardsTakenTotal.FindOrAdd( card->CardID );
			takenCount++;
		}
	}

	FLogWaveData* waveData = GetWaveDataForCards( CurrentWaveNumber_ );
	if ( waveData )
	{
		waveData->CardSelection.CardsTaken = takenIds;
	}
}

// Damage Handler (dispatcher)

void USessionLoggerSubsystem::HandleDamageDealt( AActor* instigator, AActor* target, int damage, bool bIsSplash )
{
	if ( !bIsLogging_ || !instigator || !target )
	{
		return;
	}

	if ( ADefensiveBuilding* tower = Cast<ADefensiveBuilding>( instigator ) )
	{
		AccumulateTowerDamage( tower, target, damage, bIsSplash );
	}

	if ( AUnit* enemy = Cast<AUnit>( instigator ) )
	{
		AccumulateEnemyDamage( enemy, target, damage );
	}
}

// Tower Damage Accumulation

void USessionLoggerSubsystem::AccumulateTowerDamage(
    ADefensiveBuilding* tower, AActor* target, int damage, bool bIsSplash
)
{
	FName instigatorClass = tower->GetClass()->GetFName();
	FLogDamageAccumulator& acc = TowerDamageMap_.FindOrAdd( instigatorClass );
	acc.InstigatorClass = instigatorClass;

	if ( acc.AttackType.IsEmpty() )
	{
		acc.AttackType = tower->Stats().AttackRange() >= RangedAttackThreshold_ ? TEXT( "Ranged" ) : TEXT( "Melee" );
	}

	if ( bIsSplash )
	{
		acc.SplashDamage += damage;
	}
	else
	{
		acc.DirectDamage += damage;
		acc.ShotsTotal++;
		acc.ShotsHit++;
	}

	AUnit* targetUnit = Cast<AUnit>( target );
	if ( !targetUnit )
	{
		return;
	}

	EnemyLastTowerMap_.Add( targetUnit, instigatorClass );

	if ( !targetUnit->Stats().IsAlive() )
	{
		FName enemyClass = targetUnit->GetClass()->GetFName();

		FLogEnemyDamageAccumulator& enemyAcc = EnemyDamageMap_.FindOrAdd( enemyClass );
		enemyAcc.EnemyClass = enemyClass;
		enemyAcc.Killed++;

		acc.KillCount++;

		if ( GridManager_.IsValid() )
		{
			AMainBase* base = nullptr;
			UWorld* world = GetWorld();
			if ( world )
			{
				for ( TActorIterator<AMainBase> baseIt( world ); baseIt; ++baseIt )
				{
					base = *baseIt;
					break;
				}
			}

			if ( base )
			{
				const float distToBase = FVector::Dist2D( targetUnit->GetActorLocation(), base->GetActorLocation() );
				const float maxDist = GridManager_->GetGridWidth() * cCellSizeCm * cPathProgressDistanceScale;
				const float progress = FMath::Clamp( 1.0f - ( distToBase / FMath::Max( maxDist, 1.0f ) ), 0.0f, 1.0f );
				enemyAcc.PathProgressValues.Add( progress );
			}
		}
	}
}

// Enemy Damage Accumulation

void USessionLoggerSubsystem::AccumulateEnemyDamage( AUnit* enemy, AActor* target, int damage )
{
	FName instigatorClass = enemy->GetClass()->GetFName();
	FLogEnemyDamageAccumulator& enemyAcc = EnemyDamageMap_.FindOrAdd( instigatorClass );
	enemyAcc.EnemyClass = instigatorClass;
	enemyAcc.TotalDamage += damage;

	FString targetClassName = target->GetClass()->GetName();
	if ( targetClassName.Contains( TEXT( "Wall" ) ) )
	{
		enemyAcc.DamageToWalls += damage;
	}
	else if ( Cast<ADefensiveBuilding>( target ) )
	{
		enemyAcc.DamageToDefensive += damage;
	}
	else if ( Cast<ABuilding>( target ) )
	{
		enemyAcc.DamageToEconomic += damage;
	}

	if ( ABuilding* targetBuilding = Cast<ABuilding>( target ) )
	{
		LastAttackerMap_.Add( targetBuilding, instigatorClass );
	}
}

// Bonus Helpers

void USessionLoggerSubsystem::ForEachBonusApplication(
    ABuilding* building, TFunctionRef<void( const FBonusApplication&, const FBuildingBonusEntry& )> callback
) const
{
	if ( !building )
	{
		return;
	}

	UBuildingBonusComponent* bonusComp = building->FindComponentByClass<UBuildingBonusComponent>();
	if ( !bonusComp )
	{
		return;
	}

	const TArray<FBonusApplication>& apps = bonusComp->GetActiveApplications();
	const TArray<FBuildingBonusEntry>& entries = bonusComp->GetBonusEntries();

	for ( const FBonusApplication& app : apps )
	{
		if ( app.EntryIndex_ >= 0 && app.EntryIndex_ < entries.Num() )
		{
			callback( app, entries[app.EntryIndex_] );
		}
	}
}

FLogBonusRecord USessionLoggerSubsystem::MakeBonusRecord(
    const FBuildingBonusEntry& entry, FName targetClass, const FIntPoint& targetCell, float value
) const
{
	FLogBonusRecord rec;
	rec.SourceBuildingClass = entry.SourceBuildingClass ? entry.SourceBuildingClass->GetFName() : NAME_None;
	rec.TargetBuildingClass = targetClass;
	rec.TargetCell = targetCell;
	rec.Value = value;

	switch ( entry.Category )
	{
	case EBonusCategory::Production:
		rec.Category = TEXT( "Production" );
		rec.ResourceOrStat = UEnum::GetValueAsString( entry.ResourceType );
		break;
	case EBonusCategory::Maintenance:
		rec.Category = TEXT( "Maintenance" );
		rec.ResourceOrStat = UEnum::GetValueAsString( entry.ResourceType );
		break;
	case EBonusCategory::Stats:
		rec.Category = TEXT( "Stats" );
		rec.ResourceOrStat = UEnum::GetValueAsString( entry.StatType );
		break;
	}

	return rec;
}

// Data Capture Methods

FLogResourceSnapshot USessionLoggerSubsystem::CaptureCurrentResources() const
{
	FLogResourceSnapshot snap;
	if ( ResourceManager_.IsValid() )
	{
		snap.Gold = ResourceManager_->GetResourceAmount( EResourceType::Gold );
		snap.Food = ResourceManager_->GetResourceAmount( EResourceType::Food );
		snap.Population = ResourceManager_->GetResourceAmount( EResourceType::Population );
		snap.Progress = ResourceManager_->GetResourceAmount( EResourceType::Progress );
	}
	return snap;
}

FLogBuildMapSnapshot USessionLoggerSubsystem::CaptureBuildMapState( int32 waveNumber ) const
{
	FLogBuildMapSnapshot snapshot;
	snapshot.WaveNumber = waveNumber;

	if ( !GridManager_.IsValid() )
	{
		return snapshot;
	}

	AGridManager* grid = GridManager_.Get();
	snapshot.Height = grid->GetGridHeight();
	snapshot.Width = grid->GetGridWidth();

	for ( int32 y = 0; y < snapshot.Height; ++y )
	{
		const int32 rowWidth = grid->GetRowWidth( y );
		for ( int32 x = 0; x < rowWidth; ++x )
		{
			const FGridCell* cell = grid->GetCell( x, y );
			if ( !cell )
			{
				continue;
			}

			FLogCellState cellState;
			cellState.Coords = FIntPoint( x, y );
			cellState.bIsOccupied = cell->bIsOccupied;
			cellState.bIsBuildable = cell->bIsBuildable;
			cellState.bIsWalkable = cell->bIsWalkable;
			cellState.BuildBonus = cell->BuildBonus;

			if ( cell->bIsOccupied && cell->Occupant.IsValid() )
			{
				ABuilding* bldg = cell->Occupant.Get();
				cellState.OccupantClass = bldg->GetClass()->GetFName();

				ForEachBonusApplication(
				    bldg,
				    [&]( const FBonusApplication& app, const FBuildingBonusEntry& entry )
				    {
					    cellState.ActiveBonuses.Add(
					        MakeBonusRecord( entry, cellState.OccupantClass, cellState.Coords, app.AppliedValue_ )
					    );
				    }
				);
			}

			snapshot.Cells.Add( cellState );
		}
	}

	return snapshot;
}

TArray<FLogBuildingCharacteristics> USessionLoggerSubsystem::CaptureAllBuildingStats() const
{
	TArray<FLogBuildingCharacteristics> result;

	UWorld* world = GetWorld();
	if ( !world )
	{
		return result;
	}

	for ( TActorIterator<ABuilding> it( world ); it; ++it )
	{
		ABuilding* bldg = *it;
		if ( !bldg || bldg->IsRuined() )
		{
			continue;
		}

		FLogBuildingCharacteristics chars;
		chars.BuildingClass = bldg->GetClass()->GetFName();
		chars.Category = GetBuildingCategory( bldg );

		if ( GridManager_.IsValid() )
		{
			chars.CellCoords = GridManager_->GetClosestCellCoords( bldg->GetActorLocation() );
		}

		// Get stats (with cards but INCLUDING adjacency) then subtract adjacency
		FEntityStats stats = bldg->Stats();

		ForEachBonusApplication(
		    bldg,
		    [&]( const FBonusApplication& app, const FBuildingBonusEntry& entry )
		    {
			    if ( entry.Category == EBonusCategory::Stats )
			    {
				    stats.AddStat( entry.StatType, -app.AppliedValue_ );
			    }
		    }
		);

		chars.MaxHealth = stats.MaxHealth();
		chars.AttackDamage = stats.AttackDamage();
		chars.AttackRange = stats.AttackRange();
		chars.AttackCooldown = stats.AttackCooldown();
		chars.MaxSpeed = stats.MaxSpeed();
		chars.SplashRadius = stats.SplashRadius();
		chars.BurstCount = stats.BurstCount();
		chars.BurstDelay = stats.BurstDelay();

		// Maintenance cost minus adjacency maintenance bonuses
		FResourceProduction maint = bldg->GetMaintenanceCost();
		ForEachBonusApplication(
		    bldg,
		    [&]( const FBonusApplication& app, const FBuildingBonusEntry& entry )
		    {
			    if ( entry.Category == EBonusCategory::Maintenance )
			    {
				    maint.ModifyByType( entry.ResourceType, static_cast<int32>( -app.AppliedValue_ ) );
			    }
		    }
		);
		chars.MaintenanceCost = FLogResourceSnapshot::FromProduction( maint );
		chars.BuildingCost = FLogResourceSnapshot::FromProduction( bldg->GetBuildingCost() );

		// Production minus adjacency production bonuses (resource buildings only)
		if ( AResourceBuilding* resBldg = Cast<AResourceBuilding>( bldg ) )
		{
			FResourceProduction prod = resBldg->GetProductionConfig();
			ForEachBonusApplication(
			    bldg,
			    [&]( const FBonusApplication& app, const FBuildingBonusEntry& entry )
			    {
				    if ( entry.Category == EBonusCategory::Production )
				    {
					    prod.ModifyByType( entry.ResourceType, static_cast<int32>( -app.AppliedValue_ ) );
				    }
			    }
			);
			chars.Production = FLogResourceSnapshot::FromProduction( prod );
		}

		result.Add( chars );
	}

	return result;
}

TArray<FLogBuildingCharacteristics> USessionLoggerSubsystem::CaptureDefensiveBuildingStats() const
{
	TArray<FLogBuildingCharacteristics> all = CaptureAllBuildingStats();
	TArray<FLogBuildingCharacteristics> defOnly;
	for ( const FLogBuildingCharacteristics& c : all )
	{
		if ( c.Category == TEXT( "Defensive" ) )
		{
			defOnly.Add( c );
		}
	}
	return defOnly;
}

void USessionLoggerSubsystem::CaptureEnemySpawnData( int32 waveIndex )
{
	FLogWaveData* waveData = GetCurrentWaveData();
	if ( !waveData || !WaveManager_.IsValid() )
	{
		return;
	}

	const TArray<FWave>& waves = WaveManager_->Waves;
	if ( !waves.IsValidIndex( waveIndex ) )
	{
		return;
	}

	const FWave& wave = waves[waveIndex];
	for ( int32 i = 0; i < wave.EnemyGroups.Num(); ++i )
	{
		const FEnemyGroup& group = wave.EnemyGroups[i];
		if ( !group.IsValid() )
		{
			continue;
		}

		FLogEnemySpawnRecord record;
		record.EnemyClass = group.EnemyClass ? group.EnemyClass->GetFName() : NAME_None;
		record.Count = group.Count;
		record.SpawnPointId = wave.GetSpawnPointIdForGroup( i );
		record.GroupIndex = i;

		waveData->EnemiesSpawned.Add( record );

		if ( !group.EnemyClass )
		{
			continue;
		}

		const AUnit* cdo = group.EnemyClass->GetDefaultObject<AUnit>();
		if ( !cdo )
		{
			continue;
		}

		// Skip if we already have stats for this class
		bool bAlreadyHas = false;
		for ( const FLogEnemyTypeStats& existing : waveData->EnemyTypeStats )
		{
			if ( existing.EnemyClass == record.EnemyClass )
			{
				bAlreadyHas = true;
				break;
			}
		}
		if ( bAlreadyHas )
		{
			continue;
		}

		FLogEnemyTypeStats eStats;
		eStats.EnemyClass = record.EnemyClass;

		const FEntityStats& unitStats = const_cast<AUnit*>( cdo )->Stats();
		eStats.MaxHealth = unitStats.MaxHealth();
		eStats.AttackDamage = unitStats.AttackDamage();
		eStats.AttackRange = unitStats.AttackRange();
		eStats.AttackCooldown = unitStats.AttackCooldown();
		eStats.MaxSpeed = unitStats.MaxSpeed();
		eStats.SplashRadius = unitStats.SplashRadius();
		eStats.BurstCount = unitStats.BurstCount();

		const FEnemyBuff* buff = WaveManager_->EnemyBuffs.Find( group.EnemyClass );
		if ( buff )
		{
			eStats.MaxHealth = static_cast<int32>( eStats.MaxHealth * buff->HealthMultiplier );
			eStats.AttackDamage = static_cast<int32>( eStats.AttackDamage * buff->AttackDamageMultiplier );
			eStats.AttackRange *= buff->AttackRangeMultiplier;
			eStats.AttackCooldown *= buff->AttackCooldownMultiplier;
			eStats.MaxSpeed *= buff->MaxSpeedMultiplier;
		}

		waveData->EnemyTypeStats.Add( eStats );
	}
}

void USessionLoggerSubsystem::CollectBonusDataForBuilding(
    ABuilding* building, const FIntPoint& buildingCoords, TArray<FLogBonusRecord>& OutReceived,
    TArray<FLogBonusRecord>& OutGiven
)
{
	if ( !building )
	{
		return;
	}

	ForEachBonusApplication(
	    building,
	    [&]( const FBonusApplication& app, const FBuildingBonusEntry& entry )
	    {
		    FLogBonusRecord rec;
		    rec.SourceBuildingClass = building->GetClass()->GetFName();
		    rec.SourceCell = buildingCoords;
		    rec.Value = app.AppliedValue_;

		    if ( app.TargetBuilding_.IsValid() )
		    {
			    rec.TargetBuildingClass = app.TargetBuilding_->GetClass()->GetFName();
			    if ( GridManager_.IsValid() )
			    {
				    rec.TargetCell = GridManager_->GetClosestCellCoords( app.TargetBuilding_->GetActorLocation() );
			    }
		    }

		    switch ( entry.Category )
		    {
		    case EBonusCategory::Production:
			    rec.Category = TEXT( "Production" );
			    rec.ResourceOrStat = UEnum::GetValueAsString( entry.ResourceType );
			    break;
		    case EBonusCategory::Maintenance:
			    rec.Category = TEXT( "Maintenance" );
			    rec.ResourceOrStat = UEnum::GetValueAsString( entry.ResourceType );
			    break;
		    case EBonusCategory::Stats:
			    rec.Category = TEXT( "Stats" );
			    rec.ResourceOrStat = UEnum::GetValueAsString( entry.StatType );
			    break;
		    }

		    if ( app.TargetBuilding_.Get() == building )
		    {
			    OutReceived.Add( rec );
		    }
		    else
		    {
			    OutGiven.Add( rec );
		    }
	    }
	);
}

void USessionLoggerSubsystem::CaptureBuildingHealthSnapshot()
{
	BuildingHealthAtCombatStart_.Reset();

	UWorld* world = GetWorld();
	if ( !world )
	{
		return;
	}

	for ( TActorIterator<ABuilding> it( world ); it; ++it )
	{
		ABuilding* bldg = *it;
		if ( bldg && !bldg->IsRuined() )
		{
			BuildingHealthAtCombatStart_.Add( bldg, bldg->Stats().Health() );
		}
	}
}

FLogCombatSummary USessionLoggerSubsystem::BuildCombatSummary()
{
	FLogCombatSummary summary;
	summary.WaveDurationSeconds = static_cast<float>( FPlatformTime::Seconds() - CombatStartTime_ );

	UWorld* world = GetWorld();
	if ( !world )
	{
		return summary;
	}

	for ( TActorIterator<AMainBase> it( world ); it; ++it )
	{
		AMainBase* base = *it;
		if ( base )
		{
			const int32* startHP = BuildingHealthAtCombatStart_.Find( base );
			summary.MainBaseHealthStart = startHP ? *startHP : base->Stats().MaxHealth();
			summary.MainBaseHealthEnd = FMath::Max( 0, base->Stats().Health() );
			summary.MainBaseDamageReceived =
			    static_cast<float>( summary.MainBaseHealthStart - summary.MainBaseHealthEnd );
			break;
		}
	}

	for ( const auto& pair : BuildingHealthAtCombatStart_ )
	{
		ABuilding* bldg = pair.Key.Get();
		if ( !bldg )
		{
			continue;
		}

		const int32 hpBefore = pair.Value;
		const int32 hpAfter = bldg->IsRuined() ? 0 : bldg->Stats().Health();
		const float dmgReceived = static_cast<float>( hpBefore - hpAfter );

		if ( dmgReceived > 0.0f )
		{
			FLogBuildingDamageRecord dmgRec;
			dmgRec.BuildingClass = bldg->GetClass()->GetFName();
			dmgRec.HealthBefore = hpBefore;
			dmgRec.HealthAfter = hpAfter;
			dmgRec.DamageReceived = dmgReceived;
			if ( GridManager_.IsValid() )
			{
				dmgRec.CellCoords = GridManager_->GetClosestCellCoords( bldg->GetActorLocation() );
			}
			summary.BuildingsDamaged.Add( dmgRec );
		}
	}

	return summary;
}

FLogWaveMetrics USessionLoggerSubsystem::CalculateWaveMetrics() const
{
	FLogWaveMetrics metrics;

	const FLogWaveData* waveData = GetCurrentWaveData();
	if ( !waveData )
	{
		return metrics;
	}

	metrics.EnemyDiversityIndex = waveData->EnemyTypeStats.Num();
	for ( const FLogEnemyTypeStats& ets : waveData->EnemyTypeStats )
	{
		metrics.TotalEnemyHP += ets.TotalCount * ets.MaxHealth;
		if ( ets.AttackCooldown > 0.0f )
		{
			metrics.TotalEnemyDPS += ets.TotalCount * ( static_cast<float>( ets.AttackDamage ) / ets.AttackCooldown );
		}
	}

	// Tower DPS from last build turn's defensive stats
	for ( int32 i = waveData->Turns.Num() - 1; i >= 0; --i )
	{
		const FLogTurnData& turn = waveData->Turns[i];
		if ( turn.TurnType == TEXT( "Build" ) && turn.Defensive.AllDefensiveStats.Num() > 0 )
		{
			for ( const FLogBuildingCharacteristics& defStat : turn.Defensive.AllDefensiveStats )
			{
				if ( defStat.AttackCooldown > 0.0f )
				{
					metrics.TotalTowerDPS += static_cast<float>( defStat.AttackDamage ) / defStat.AttackCooldown;
				}
			}
			break;
		}
	}

	if ( metrics.TotalEnemyHP > 0 )
	{
		metrics.DpsToHpRatio = metrics.TotalTowerDPS / static_cast<float>( metrics.TotalEnemyHP );
	}

	if ( metrics.TotalTowerDPS > 0.0f )
	{
		const float ratio = metrics.TotalEnemyDPS / metrics.TotalTowerDPS;
		metrics.DifficultyScore = FMath::Clamp( ratio * cDifficultyScoreMultiplier, 1.0f, 10.0f );
	}
	else
	{
		metrics.DifficultyScore = 10.0f;
	}

	return metrics;
}

void USessionLoggerSubsystem::CalculateSessionMetrics()
{
	SessionData_.TotalSessionDurationSeconds = static_cast<float>( FPlatformTime::Seconds() - SessionStartTime_ );
	SessionData_.ClosestCallWave = ClosestCallWave_;

	int32 totalKilled = 0;
	for ( const FLogWaveData& wave : SessionData_.Waves )
	{
		for ( const FLogEnemyTypeStats& ets : wave.EnemyTypeStats )
		{
			totalKilled += ets.Killed;
		}
	}
	SessionData_.TotalEnemiesKilled = totalKilled;
}

FString USessionLoggerSubsystem::GetBuildingCategory( const ABuilding* building ) const
{
	if ( Cast<AMainBase>( building ) )
	{
		return TEXT( "MainBase" );
	}
	if ( Cast<ADefensiveBuilding>( building ) )
	{
		return TEXT( "Defensive" );
	}
	if ( Cast<AResourceBuilding>( building ) )
	{
		return TEXT( "Resource" );
	}
	if ( Cast<AAdditiveBuilding>( building ) )
	{
		return TEXT( "Additive" );
	}
	return TEXT( "Other" );
}

// Wave/Turn Lifecycle

void USessionLoggerSubsystem::BeginNewWave( int32 waveNumber )
{
	CurrentWaveIndex_ = INDEX_NONE;
	CurrentTurnIndex_ = INDEX_NONE;

	FLogWaveData newWave;
	newWave.WaveNumber = waveNumber;
	newWave.Turns.Reserve( cExpectedTurnsPerWave );
	SessionData_.Waves.Add( newWave );
	CurrentWaveIndex_ = SessionData_.Waves.Num() - 1;
}

void USessionLoggerSubsystem::BeginNewTurn( int32 turnNumber, const FString& turnType )
{
	FLogWaveData* waveData = GetCurrentWaveData();
	if ( !waveData )
	{
		return;
	}

	FLogTurnData newTurn;
	newTurn.TurnNumber = turnNumber;
	newTurn.TurnType = turnType;

	TurnStartTime_ = FPlatformTime::Seconds();

	ResourcesAtTurnStart_ = CaptureCurrentResources();
	newTurn.ResourcesAtStart = ResourcesAtTurnStart_;

	AccumulatedBuildCost_ = FLogResourceSnapshot();
	AccumulatedDefensiveBuildCost_ = FLogResourceSnapshot();

	waveData->Turns.Add( newTurn );
	CurrentTurnIndex_ = waveData->Turns.Num() - 1;
}

// EndCurrentTurn (orchestrator)

void USessionLoggerSubsystem::EndCurrentTurn()
{
	FLogTurnData* turnData = GetCurrentTurnData();
	if ( !turnData )
	{
		return;
	}

	// Timing
	turnData->TurnDurationSeconds = static_cast<float>( FPlatformTime::Seconds() - TurnStartTime_ );

	// Build cost
	turnData->SpentOnBuilding = AccumulatedBuildCost_;
	SessionData_.TotalSpentOnBuilding += AccumulatedBuildCost_;

	// Resources at end
	turnData->ResourcesAtEnd = CaptureCurrentResources();

	// Economy data (maintenance, cards, adjacency, income)
	CaptureTurnEconomy();

	// Building characteristics (re-fetch turnData since CaptureTurnEconomy may have been safe but be careful)
	turnData = GetCurrentTurnData();
	if ( turnData )
	{
		turnData->AllBuildingCharacteristics = CaptureAllBuildingStats();
	}

	// Defensive data
	CaptureDefensiveTurnData();

	// Combat data (only for combat turns)
	CaptureCombatTurnData();

	CurrentTurnIndex_ = INDEX_NONE;
}

// Turn Economy Capture

void USessionLoggerSubsystem::CaptureTurnEconomy()
{
	FLogTurnData* turnData = GetCurrentTurnData();
	if ( !turnData )
	{
		return;
	}

	UWorld* world = GetWorld();
	if ( !world )
	{
		return;
	}

	// Total maintenance from all buildings
	FLogResourceSnapshot totalMaint;
	for ( TActorIterator<ABuilding> maintIt( world ); maintIt; ++maintIt )
	{
		ABuilding* bldg = *maintIt;
		if ( bldg && !bldg->IsRuined() )
		{
			totalMaint += FLogResourceSnapshot::FromProduction( bldg->GetMaintenanceCost() );
		}
	}

	// Card-based economy bonuses
	if ( UCardSubsystem* cardSub = UCardSubsystem::Get( this ) )
	{
		const FEconomyBonuses& bonuses = cardSub->GetEconomyBonuses();

		turnData->CardProductionBonus.Gold = bonuses.GoldProductionBonus;
		turnData->CardProductionBonus.Food = bonuses.FoodProductionBonus;
		turnData->CardProductionBonus.Population = bonuses.PopulationProductionBonus;
		turnData->CardProductionBonus.Progress = bonuses.ProgressProductionBonus;

		turnData->CardMaintenanceReduction.Gold = FMath::Min( totalMaint.Gold, bonuses.GoldMaintenanceReduction );
		turnData->CardMaintenanceReduction.Food = FMath::Min( totalMaint.Food, bonuses.FoodMaintenanceReduction );
		turnData->CardMaintenanceReduction.Population =
		    FMath::Min( totalMaint.Population, bonuses.PopulationMaintenanceReduction );
		turnData->CardMaintenanceReduction.Progress =
		    FMath::Min( totalMaint.Progress, bonuses.ProgressMaintenanceReduction );

		totalMaint.Gold = FMath::Max( 0, totalMaint.Gold - bonuses.GoldMaintenanceReduction );
		totalMaint.Food = FMath::Max( 0, totalMaint.Food - bonuses.FoodMaintenanceReduction );
		totalMaint.Population = FMath::Max( 0, totalMaint.Population - bonuses.PopulationMaintenanceReduction );
		totalMaint.Progress = FMath::Max( 0, totalMaint.Progress - bonuses.ProgressMaintenanceReduction );
	}
	turnData->MaintenancePaid = totalMaint;

	// Adjacency bonus income (production bonuses on resource buildings)
	FLogResourceSnapshot adjBonusIncome;
	for ( TActorIterator<AResourceBuilding> resIt( world ); resIt; ++resIt )
	{
		AResourceBuilding* resBldg = *resIt;
		if ( !resBldg || resBldg->IsRuined() )
		{
			continue;
		}

		ForEachBonusApplication(
		    resBldg,
		    [&]( const FBonusApplication& app, const FBuildingBonusEntry& entry )
		    {
			    if ( app.TargetBuilding_.Get() != resBldg )
			    {
				    return;
			    }
			    if ( entry.Category == EBonusCategory::Production )
			    {
				    FResourceProduction bonusProd;
				    bonusProd.ModifyByType( entry.ResourceType, static_cast<int32>( app.AppliedValue_ ) );
				    adjBonusIncome += FLogResourceSnapshot::FromProduction( bonusProd );
			    }
		    }
		);
	}
	turnData->AdjacencyBonusIncome = adjBonusIncome;

	// Computed income from resource balance equation
	FLogResourceSnapshot computedIncome = turnData->ResourcesAtEnd - turnData->ResourcesAtStart +
	                                      turnData->SpentOnBuilding + turnData->MaintenancePaid -
	                                      turnData->AdjacencyBonusIncome - turnData->CardProductionBonus;

	computedIncome.Gold = FMath::Max( 0, computedIncome.Gold );
	computedIncome.Food = FMath::Max( 0, computedIncome.Food );
	computedIncome.Population = FMath::Max( 0, computedIncome.Population );
	computedIncome.Progress = FMath::Max( 0, computedIncome.Progress );
	turnData->IncomeFromTransition = computedIncome;
}

// Defensive Turn Data Capture

void USessionLoggerSubsystem::CaptureDefensiveTurnData()
{
	FLogTurnData* turnData = GetCurrentTurnData();
	if ( !turnData )
	{
		return;
	}

	UWorld* world = GetWorld();
	if ( !world )
	{
		return;
	}

	turnData->Defensive.SpentOnDefensive = AccumulatedDefensiveBuildCost_;

	// Defensive maintenance
	FLogResourceSnapshot defMaint;
	for ( TActorIterator<ADefensiveBuilding> it( world ); it; ++it )
	{
		ADefensiveBuilding* def = *it;
		if ( def && !def->IsRuined() )
		{
			defMaint += FLogResourceSnapshot::FromProduction( def->GetMaintenanceCost() );
		}
	}
	turnData->Defensive.DefensiveMaintenance = defMaint;

	// Defensive building stats (without adjacency)
	turnData->Defensive.AllDefensiveStats = CaptureDefensiveBuildingStats();

	// Adjacency bonuses for defensive buildings
	for ( TActorIterator<ADefensiveBuilding> it( world ); it; ++it )
	{
		ADefensiveBuilding* def = *it;
		if ( !def || def->IsRuined() )
		{
			continue;
		}

		FIntPoint defCell = FIntPoint( -1, -1 );
		if ( GridManager_.IsValid() )
		{
			defCell = GridManager_->GetClosestCellCoords( def->GetActorLocation() );
		}

		ForEachBonusApplication(
		    def,
		    [&]( const FBonusApplication& app, const FBuildingBonusEntry& entry )
		    {
			    if ( app.TargetBuilding_.Get() == def )
			    {
				    turnData->Defensive.AdjacencyBonuses.Add(
				        MakeBonusRecord( entry, def->GetClass()->GetFName(), defCell, app.AppliedValue_ )
				    );
			    }
		    }
		);
	}
}

// Combat Turn Data Capture

void USessionLoggerSubsystem::CaptureCombatTurnData()
{
	FLogTurnData* turnData = GetCurrentTurnData();
	if ( !turnData || turnData->TurnType != TEXT( "Combat" ) )
	{
		return;
	}

	// Tower damage records
	for ( const auto& pair : TowerDamageMap_ )
	{
		FLogTowerDamageRecord dmgRecord;
		dmgRecord.TowerClass = pair.Value.InstigatorClass;
		dmgRecord.AttackType = pair.Value.AttackType;
		dmgRecord.DirectDamage = pair.Value.DirectDamage;
		dmgRecord.SplashDamage = pair.Value.SplashDamage;
		dmgRecord.TotalDamage = pair.Value.DirectDamage + pair.Value.SplashDamage;
		dmgRecord.KillCount = pair.Value.KillCount;
		dmgRecord.ShotsTotal = pair.Value.ShotsTotal;
		dmgRecord.ShotsHit = pair.Value.ShotsHit;
		turnData->Defensive.TowerDamage.Add( dmgRecord );
	}

	// Combat summary
	turnData->Defensive.CombatSummary = BuildCombatSummary();

	// Track closest-call wave (lowest base HP)
	const int32 baseHPAfter = turnData->Defensive.CombatSummary.MainBaseHealthEnd;
	if ( baseHPAfter < LowestBaseHP_ )
	{
		LowestBaseHP_ = baseHPAfter;
		ClosestCallWave_ = CurrentWaveNumber_;
	}
}

// FinalizeWave

void USessionLoggerSubsystem::FinalizeWave()
{
	FLogWaveData* waveData = GetCurrentWaveData();
	if ( !waveData )
	{
		return;
	}

	// Aggregate spawn counts and spawn points per enemy type
	for ( FLogEnemyTypeStats& ets : waveData->EnemyTypeStats )
	{
		TSet<FName> uniqueSpawnPoints;
		for ( const FLogEnemySpawnRecord& spawn : waveData->EnemiesSpawned )
		{
			if ( spawn.EnemyClass == ets.EnemyClass )
			{
				ets.TotalCount += spawn.Count;
				if ( !spawn.SpawnPointId.IsNone() )
				{
					uniqueSpawnPoints.Add( spawn.SpawnPointId );
				}
			}
		}
		ets.SpawnPoints = uniqueSpawnPoints.Array();
	}

	// Fill enemy damage and penetration stats
	for ( FLogEnemyTypeStats& ets : waveData->EnemyTypeStats )
	{
		FLogEnemyDamageAccumulator* acc = EnemyDamageMap_.Find( ets.EnemyClass );
		if ( acc )
		{
			ets.DamageToDefensive = acc->DamageToDefensive;
			ets.DamageToEconomic = acc->DamageToEconomic;
			ets.DamageToWalls = acc->DamageToWalls;
			ets.TotalDamage = acc->TotalDamage;
			ets.BuildingsDestroyedCounts = acc->BuildingsDestroyedCounts;

			ets.Killed = acc->Killed;
			ets.ReachedBase = ets.TotalCount - acc->Killed;
			if ( acc->PathProgressValues.Num() > 0 )
			{
				float sum = 0.0f;
				float maxVal = 0.0f;
				for ( float p : acc->PathProgressValues )
				{
					sum += p;
					maxVal = FMath::Max( maxVal, p );
				}
				ets.AvgPathProgress = sum / acc->PathProgressValues.Num();
				ets.MaxPathProgress = maxVal;
			}
		}
	}

	waveData->WaveMetrics = CalculateWaveMetrics();
	waveData->BuildMap = CaptureBuildMapState( waveData->WaveNumber );

	// Calculate totals for income/maintenance
	FLogResourceSnapshot waveIncome;
	FLogResourceSnapshot waveMaintenance;
	for ( const FLogTurnData& turn : waveData->Turns )
	{
		waveIncome += turn.IncomeFromTransition;
		waveMaintenance += turn.MaintenancePaid;
	}
	SessionData_.TotalIncome += waveIncome;
	SessionData_.TotalMaintenancePaid += waveMaintenance;

	OnWaveDataFinalized.Broadcast( *waveData );

	// NOTE: Do NOT reset CurrentWaveIndex_ here — card selection events
	// fire during the Reward phase (after FinalizeWave) and need to
	// write to the current wave. CurrentWaveIndex_ is reset in BeginNewWave.
}

// FinalizeSession

void USessionLoggerSubsystem::FinalizeSession( bool bVictory )
{
	if ( !bIsLogging_ )
	{
		return;
	}

	if ( GetCurrentTurnData() )
	{
		EndCurrentTurn();
	}

	if ( GetCurrentWaveData() )
	{
		FinalizeWave();
	}

	SessionData_.bVictory = bVictory;
	SessionData_.WavesSurvived = CurrentWaveNumber_;
	SessionOutcome_ = bVictory ? TEXT( "Victory" ) : TEXT( "Defeat" );

	SessionData_.FinalFieldState = CaptureBuildMapState( CurrentWaveNumber_ );

	CalculateSessionMetrics();

	for ( ISessionDataCollector* collector : DataCollectors_ )
	{
		if ( collector )
		{
			collector->OnGameEnded( bVictory );
		}
	}

	WriteSessionToFile();

	OnSessionFinalized.Broadcast( SessionData_ );

	bIsLogging_ = false;
}

// FinalizeSessionOnRestart

void USessionLoggerSubsystem::FinalizeSessionOnRestart()
{
	// Diagnostic: append state to file next to the .exe
	if ( !bIsLogging_ )
	{
		return;
	}

	if ( GetCurrentTurnData() )
	{
		EndCurrentTurn();
	}

	if ( GetCurrentWaveData() )
	{
		FinalizeWave();
	}

	SessionData_.bVictory = false;
	SessionData_.WavesSurvived = CurrentWaveNumber_;
	SessionOutcome_ = TEXT( "Restart" );

	SessionData_.FinalFieldState = CaptureBuildMapState( CurrentWaveNumber_ );

	CalculateSessionMetrics();

	for ( ISessionDataCollector* collector : DataCollectors_ )
	{
		if ( collector )
		{
			collector->OnGameEnded( false );
		}
	}

	WriteSessionToFile();

	OnSessionFinalized.Broadcast( SessionData_ );

	bIsLogging_ = false;
}

// JSON Output (GZip compressed)

void USessionLoggerSubsystem::WriteSessionToFile()
{
	TSharedPtr<FJsonObject> rootJson = SessionData_.ToJson();

	for ( ISessionDataCollector* collector : DataCollectors_ )
	{
		if ( collector )
		{
			collector->AppendToJson( rootJson );
		}
	}

	FString jsonString;
	TSharedRef<TJsonWriter<>> writer = TJsonWriterFactory<>::Create( &jsonString );
	FJsonSerializer::Serialize( rootJson.ToSharedRef(), writer );
	writer->Close();

	const FString filePath = FPaths::ConvertRelativePathToFull( GetOutputFilePath() );

	const FString dir = FPaths::GetPath( filePath );
	IFileManager::Get().MakeDirectory( *dir, true );

	FTCHARToUTF8 utf8Converter( *jsonString );
	TArray<uint8> utf8Data;
	utf8Data.Append( reinterpret_cast<const uint8*>( utf8Converter.Get() ), utf8Converter.Length() );

	z_stream stream = {};
	if ( deflateInit2( &stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY ) != Z_OK )
	{
		UE_LOG( LogSessionLogger, Error, TEXT( "Failed to initialize zlib for gzip compression" ) );
		return;
	}

	const uLong bound = deflateBound( &stream, utf8Data.Num() );
	TArray<uint8> gzipData;
	gzipData.SetNumUninitialized( bound );

	stream.next_in = const_cast<Bytef*>( utf8Data.GetData() );
	stream.avail_in = utf8Data.Num();
	stream.next_out = gzipData.GetData();
	stream.avail_out = gzipData.Num();

	const int32 result = deflate( &stream, Z_FINISH );
	deflateEnd( &stream );

	if ( result != Z_STREAM_END )
	{
		UE_LOG( LogSessionLogger, Error, TEXT( "Gzip compression failed (zlib error %d)" ), result );
		return;
	}

	gzipData.SetNum( stream.total_out );

	if ( FFileHelper::SaveArrayToFile( gzipData, *filePath ) )
	{
		UE_LOG( LogSessionLogger, Log, TEXT( "Session log saved to: %s" ), *filePath );
	}
	else
	{
		UE_LOG( LogSessionLogger, Error, TEXT( "Failed to save session log to: %s" ), *filePath );
	}
}

FString USessionLoggerSubsystem::GetOutputFilePath() const
{
	FString mapName = SessionData_.MapName;
	if ( mapName.IsEmpty() )
	{
		mapName = TEXT( "UnknownMap" );
	}

	// Sanitize for filesystem safety
	mapName = FPaths::MakeValidFileName( mapName );

	const FString outcome = SessionOutcome_.IsEmpty()
	                            ? ( SessionData_.bVictory ? FString( TEXT( "Victory" ) ) : FString( TEXT( "Defeat" ) ) )
	                            : SessionOutcome_;
	const FString timestamp = FDateTime::Now().ToString( TEXT( "%Y-%m-%d_%H-%M-%S" ) );
	const FString fileName = FString::Printf( TEXT( "%s_%s_%s.json.gz" ), *mapName, *outcome, *timestamp );
	return FPaths::ProjectDir() / TEXT( "SessionLogs" ) / fileName;
}