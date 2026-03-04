#include "Core/Subsystems/SessionLogger/SessionLoggerSubsystem.h"

#include "Core/Subsystems/SessionLogger/DamageEvent.h"
#include "Core/Subsystems/SessionLogger/ISessionDataCollector.h"
#include "Core/CoreManager.h"
#include "Core/GameLoopManager.h"
#include "Building/Building.h"
#include "Building/DefensiveBuilding.h"
#include "Building/ResourceBuilding.h"
#include "Building/AdditiveBuilding.h"
#include "Building/MainBase.h"
#include "Building/Construction/BuildManager.h"
#include "Building/Bonus/BuildingBonusComponent.h"
#include "Cards/CardSubsystem.h"
#include "Cards/CardDataAsset.h"
#include "Grid/GridManager.h"
#include "Grid/GridCell.h"
#include "Resources/ResourceManager.h"
#include "Resources/EconomyComponent.h"
#include "Waves/WaveManager.h"
#include "Waves/Wave.h"
#include "Waves/EnemyGroup.h"
#include "Waves/EnemyBuff.h"
#include "Units/Unit.h"
#include "Components/Attack/AttackRangedComponent.h"
#include "Components/Attack/AttackMeleeComponent.h"

#include "Async/Async.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "EngineUtils.h"

DEFINE_LOG_CATEGORY_STATIC( LogSessionLogger, Log, All );

// ============================================================================
// Lifecycle
// ============================================================================

void USessionLoggerSubsystem::Initialize( FSubsystemCollectionBase& Collection )
{
	Super::Initialize( Collection );

	// Pre-allocate
	SessionData_.Waves.Reserve( 20 );

	// Delay binding until systems are ready
	if ( UCoreManager* Core = UCoreManager::Get( this ) )
	{
		Core->OnSystemsReady.AddDynamic( this, &USessionLoggerSubsystem::OnCoreSystemsReady );
	}
}

void USessionLoggerSubsystem::OnCoreSystemsReady()
{
	BindToSystems();
}

void USessionLoggerSubsystem::Deinitialize()
{
	if ( UCoreManager* Core = UCoreManager::Get( this ) )
	{
		Core->OnSystemsReady.RemoveDynamic( this, &USessionLoggerSubsystem::OnCoreSystemsReady );
	}

	UnbindFromSystems();
	Super::Deinitialize();
}

// ============================================================================
// System Binding
// ============================================================================

void USessionLoggerSubsystem::BindToSystems()
{
	if ( bIsBound_ )
	{
		return;
	}

	UCoreManager* Core = UCoreManager::Get( this );
	if ( !Core )
	{
		return;
	}

	GameLoop_ = Core->GetGameLoop();
	WaveManager_ = Core->GetWaveManager();
	ResourceManager_ = Core->GetResourceManager();
	EconomyComponent_ = Core->GetEconomyComponent();
	GridManager_ = Core->GetGridManager();
	BuildManager_ = Core->GetBuildManager();

	if ( GameLoop_.IsValid() )
	{
		GameLoop_->OnPhaseChanged.AddDynamic( this, &USessionLoggerSubsystem::HandlePhaseChanged );
		GameLoop_->OnBuildTurnChanged.AddDynamic( this, &USessionLoggerSubsystem::HandleBuildTurnChanged );
		GameLoop_->OnWaveChanged.AddDynamic( this, &USessionLoggerSubsystem::HandleWaveChanged );
		GameLoop_->OnGameEnded.AddDynamic( this, &USessionLoggerSubsystem::HandleGameEnded );
	}

	if ( WaveManager_.IsValid() )
	{
		WaveManager_->OnWaveStarted.AddDynamic( this, &USessionLoggerSubsystem::HandleWaveStarted );
	}

	if ( BuildManager_.IsValid() )
	{
		BuildManager_->OnBuildingConfirmed.AddDynamic( this, &USessionLoggerSubsystem::HandleBuildingPlaced );
	}

	// Card subsystem
	if ( UCardSubsystem* CardSub = UCardSubsystem::Get( this ) )
	{
		CardSub->OnCardSelectionRequired.AddDynamic( this, &USessionLoggerSubsystem::HandleCardSelectionRequired );
		CardSub->OnCardsApplied.AddDynamic( this, &USessionLoggerSubsystem::HandleCardsApplied );
	}

	// Static damage delegate
	DamageEventHandle_ =
	    FDamageEvents::OnDamageDealt.AddUObject( this, &USessionLoggerSubsystem::HandleDamageDealt );

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

	if ( UCardSubsystem* CardSub = UCardSubsystem::Get( this ) )
	{
		CardSub->OnCardSelectionRequired.RemoveDynamic( this, &USessionLoggerSubsystem::HandleCardSelectionRequired );
		CardSub->OnCardsApplied.RemoveDynamic( this, &USessionLoggerSubsystem::HandleCardsApplied );
	}

	FDamageEvents::OnDamageDealt.Remove( DamageEventHandle_ );

	bIsBound_ = false;
}

// ============================================================================
// Extensibility
// ============================================================================

void USessionLoggerSubsystem::RegisterCollector( ISessionDataCollector* Collector )
{
	if ( Collector )
	{
		DataCollectors_.AddUnique( Collector );
	}
}

void USessionLoggerSubsystem::UnregisterCollector( ISessionDataCollector* Collector )
{
	DataCollectors_.Remove( Collector );
}

int32 USessionLoggerSubsystem::GetCurrentWaveNumber() const
{
	return CurrentWaveNumber_;
}

// ============================================================================
// Phase Changed Handler
// ============================================================================

void USessionLoggerSubsystem::HandlePhaseChanged( EGameLoopPhase OldPhase, EGameLoopPhase NewPhase )
{
	// Notify collectors
	for ( ISessionDataCollector* Collector : DataCollectors_ )
	{
		Collector->OnPhaseChanged( OldPhase, NewPhase );
	}

	switch ( NewPhase )
	{
	case EGameLoopPhase::Building:
	{
		if ( OldPhase == EGameLoopPhase::Startup || OldPhase == EGameLoopPhase::None )
		{
			// Game start - initialize session
			bIsLogging_ = true;
			SessionStartTime_ = FPlatformTime::Seconds();
			LowestBaseHP_ = INT32_MAX;
			ClosestCallWave_ = 0;
			SessionData_ = FLogSessionData();
			SessionData_.MapName = GetWorld()->GetMapName();
			SessionData_.MapName.RemoveFromStart( GetWorld()->StreamingLevelsPrefix );
			SessionData_.Timestamp = FDateTime::Now().ToString( TEXT( "%Y-%m-%dT%H:%M:%S" ) );

			// Capture initial field state
			SessionData_.InitialFieldState = CaptureBuildMapState( 0 );

			CurrentWaveNumber_ = 1;
			BeginNewWave( CurrentWaveNumber_ );
			BeginNewTurn( 1, TEXT( "Build" ) );
		}
		else if ( OldPhase == EGameLoopPhase::Reward )
		{
			// New wave started after reward phase
			BeginNewWave( CurrentWaveNumber_ );
			BeginNewTurn( 1, TEXT( "Build" ) );
		}
		break;
	}
	case EGameLoopPhase::Combat:
	{
		// End last build turn, start combat turn
		EndCurrentTurn();
		BeginNewTurn( 3, TEXT( "Combat" ) );
		CombatStartTime_ = FPlatformTime::Seconds();

		// Reset damage accumulators for this wave
		TowerDamageMap_.Reset();
		EnemyDamageMap_.Reset();
		LastAttackerMap_.Reset();
		EnemyLastTowerMap_.Reset();

		// Capture building HP snapshot before combat
		CaptureBuildingHealthSnapshot();

		// Subscribe to death events of all current buildings
		for ( TActorIterator<ABuilding> It( GetWorld() ); It; ++It )
		{
			ABuilding* Bldg = *It;
			if ( Bldg && !Bldg->IsRuined() )
			{
				Bldg->OnBuildingDied.AddDynamic( this, &USessionLoggerSubsystem::HandleBuildingDied );
			}
		}

		break;
	}
	case EGameLoopPhase::Reward:
	{
		// End combat turn
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

// ============================================================================
// Turn Changed Handler
// ============================================================================

void USessionLoggerSubsystem::HandleBuildTurnChanged( int32 CurrentTurn, int32 MaxTurns )
{
	// Notify collectors
	for ( ISessionDataCollector* Collector : DataCollectors_ )
	{
		Collector->OnTurnChanged( CurrentTurn, MaxTurns );
	}

	// End previous turn, begin new build turn
	EndCurrentTurn();
	BeginNewTurn( CurrentTurn, TEXT( "Build" ) );
}

// ============================================================================
// Wave Changed Handler
// ============================================================================

void USessionLoggerSubsystem::HandleWaveChanged( int32 CurrentWave, int32 TotalWaves )
{
	CurrentWaveNumber_ = CurrentWave;

	for ( ISessionDataCollector* Collector : DataCollectors_ )
	{
		Collector->OnWaveChanged( CurrentWave, TotalWaves );
	}
}

// ============================================================================
// Game Ended Handler
// ============================================================================

void USessionLoggerSubsystem::HandleGameEnded( bool bVictory )
{
	for ( ISessionDataCollector* Collector : DataCollectors_ )
	{
		Collector->OnGameEnded( bVictory );
	}

	// FinalizeSession is already called via HandlePhaseChanged (Victory/Defeat)
}

// ============================================================================
// Wave Started Handler
// ============================================================================

void USessionLoggerSubsystem::HandleWaveStarted( int32 WaveIndex )
{
	CaptureEnemySpawnData( WaveIndex );
}

// ============================================================================
// Building Placed Handler
// ============================================================================

void USessionLoggerSubsystem::HandleBuildingPlaced( ABuilding* Building, FIntPoint CellCoords )
{
	if ( !CurrentTurnData_ || !Building )
	{
		return;
	}

	FLogBuildingPlacementRecord Record;
	Record.BuildingClass = Building->GetClass()->GetFName();
	Record.Category = GetBuildingCategory( Building );
	Record.CellCoords = CellCoords;
	Record.Cost = FLogResourceSnapshot::FromProduction( Building->GetBuildingCost() );

	// Collect bonus info
	CollectBonusDataForBuilding( Building, CellCoords, Record.BonusesReceived, Record.BonusesGiven );

	CurrentTurnData_->BuildingsPlaced.Add( Record );

	// Accumulate costs
	AccumulatedBuildCost_ += Record.Cost;
	SessionData_.TotalBuildingsPlaced++;

	// Track cell popularity
	int32& Count = SessionData_.CellPopularity.FindOrAdd( CellCoords );
	Count++;

	// Defensive-specific tracking
	if ( Cast<ADefensiveBuilding>( Building ) )
	{
		AccumulatedDefensiveBuildCost_ += Record.Cost;
		CurrentTurnData_->Defensive.DefensivePlacements.Add( Record );
	}
}

// ============================================================================
// Building Died Handler
// ============================================================================

void USessionLoggerSubsystem::HandleBuildingDied( ABuilding* Building )
{
	if ( !CurrentTurnData_ || !Building )
	{
		return;
	}

	FLogBuildingDestroyedRecord Record;
	Record.BuildingClass = Building->GetClass()->GetFName();
	Record.Category = GetBuildingCategory( Building );

	// Try to find cell coords
	if ( GridManager_.IsValid() )
	{
		Record.CellCoords = GridManager_->GetClosestCellCoords( Building->GetActorLocation() );
	}

	CurrentTurnData_->BuildingsDestroyed.Add( Record );
	SessionData_.TotalBuildingsLost++;

	// Track which enemy type destroyed this building
	FName* LastAttacker = LastAttackerMap_.Find( Building );
	if ( LastAttacker )
	{
		FLogEnemyDamageAccumulator& EnemyAcc = EnemyDamageMap_.FindOrAdd( *LastAttacker );
		EnemyAcc.EnemyClass = *LastAttacker;
		EnemyAcc.BuildingsDestroyedCounts.FindOrAdd( Record.BuildingClass )++;
	}

	// Unbind from this building
	Building->OnBuildingDied.RemoveDynamic( this, &USessionLoggerSubsystem::HandleBuildingDied );
}

// ============================================================================
// Card Handlers
// ============================================================================

void USessionLoggerSubsystem::HandleCardSelectionRequired( const FCardChoice& Choice )
{
	LastOfferedCards_.Reset();
	for ( const auto& Card : Choice.AvailableCards )
	{
		if ( Card )
		{
			LastOfferedCards_.Add( Card->CardID );
			int32& ShownCount = SessionData_.CardsShownTotal.FindOrAdd( Card->CardID );
			ShownCount++;
		}
	}

	if ( CurrentWaveData_ )
	{
		CurrentWaveData_->CardSelection.WaveNumber = Choice.WaveNumber;
		CurrentWaveData_->CardSelection.CardsOffered = LastOfferedCards_;
	}
}

void USessionLoggerSubsystem::HandleCardsApplied( const TArray<UCardDataAsset*>& AppliedCards )
{
	TArray<FName> TakenIds;
	for ( const UCardDataAsset* Card : AppliedCards )
	{
		if ( Card )
		{
			TakenIds.Add( Card->CardID );
			int32& TakenCount = SessionData_.CardsTakenTotal.FindOrAdd( Card->CardID );
			TakenCount++;
		}
	}

	if ( CurrentWaveData_ )
	{
		CurrentWaveData_->CardSelection.CardsTaken = TakenIds;
	}
}

// ============================================================================
// Damage Handler
// ============================================================================

void USessionLoggerSubsystem::HandleDamageDealt( AActor* Instigator, AActor* Target, float Damage, bool bIsSplash )
{
	if ( !bIsLogging_ || !Instigator || !Target )
	{
		return;
	}

	FName InstigatorClass = Instigator->GetClass()->GetFName();

	// Tower dealing damage to enemy
	if ( ADefensiveBuilding* Tower = Cast<ADefensiveBuilding>( Instigator ) )
	{
		FLogDamageAccumulator& Acc = TowerDamageMap_.FindOrAdd( InstigatorClass );
		Acc.InstigatorClass = InstigatorClass;

		// Determine attack type
		if ( Acc.AttackType.IsEmpty() )
		{
			if ( Tower->FindComponentByClass<UAttackRangedComponent>() )
			{
				Acc.AttackType = TEXT( "Ranged" );
			}
			else
			{
				Acc.AttackType = TEXT( "Melee" );
			}
		}

		if ( bIsSplash )
		{
			Acc.SplashDamage += Damage;
		}
		else
		{
			Acc.DirectDamage += Damage;
			Acc.ShotsTotal++;
			Acc.ShotsHit++;
		}

		// Track last tower attacker on this enemy (for kill credit)
		if ( AUnit* TargetUnit = Cast<AUnit>( Target ) )
		{
			EnemyLastTowerMap_.Add( TargetUnit, InstigatorClass );

			// Detect enemy death: if HP drops to 0 after this damage
			if ( !TargetUnit->Stats().IsAlive() )
			{
				FName EnemyClass = TargetUnit->GetClass()->GetFName();

				// Track kill count per enemy type
				FLogEnemyDamageAccumulator& EnemyAcc = EnemyDamageMap_.FindOrAdd( EnemyClass );
				EnemyAcc.EnemyClass = EnemyClass;
				EnemyAcc.Killed++;

				// Credit the kill to this tower
				Acc.KillCount++;

				// Track path progress (how far the enemy got before dying)
				if ( GridManager_.IsValid() )
				{
					AMainBase* Base = nullptr;
					for ( TActorIterator<AMainBase> BaseIt( GetWorld() ); BaseIt; ++BaseIt )
					{
						Base = *BaseIt;
						break;
					}

					if ( Base )
					{
						const float DistToBase = FVector::Dist2D( TargetUnit->GetActorLocation(), Base->GetActorLocation() );
						const float MaxDist = GridManager_->GetGridWidth() * 100.0f * 1.5f;
						const float Progress = FMath::Clamp( 1.0f - ( DistToBase / FMath::Max( MaxDist, 1.0f ) ), 0.0f, 1.0f );
						EnemyAcc.PathProgressValues.Add( Progress );
					}
				}
			}
		}
	}

	// Enemy dealing damage to building
	if ( AUnit* Enemy = Cast<AUnit>( Instigator ) )
	{
		FLogEnemyDamageAccumulator& EnemyAcc = EnemyDamageMap_.FindOrAdd( InstigatorClass );
		EnemyAcc.EnemyClass = InstigatorClass;
		EnemyAcc.TotalDamage += Damage;

		// Classify damage by target type
		FString TargetClassName = Target->GetClass()->GetName();
		if ( TargetClassName.Contains( TEXT( "Wall" ) ) )
		{
			EnemyAcc.DamageToWalls += Damage;
		}
		else if ( Cast<ADefensiveBuilding>( Target ) )
		{
			EnemyAcc.DamageToDefensive += Damage;
		}
		else if ( Cast<ABuilding>( Target ) )
		{
			EnemyAcc.DamageToEconomic += Damage;
		}

		// Track last attacker per building
		if ( ABuilding* TargetBuilding = Cast<ABuilding>( Target ) )
		{
			LastAttackerMap_.Add( TargetBuilding, InstigatorClass );
		}
	}
}

// ============================================================================
// Data Capture Methods
// ============================================================================

FLogResourceSnapshot USessionLoggerSubsystem::CaptureCurrentResources() const
{
	FLogResourceSnapshot Snap;
	if ( ResourceManager_.IsValid() )
	{
		Snap.Gold = ResourceManager_->GetResourceAmount( EResourceType::Gold );
		Snap.Food = ResourceManager_->GetResourceAmount( EResourceType::Food );
		Snap.Population = ResourceManager_->GetResourceAmount( EResourceType::Population );
		Snap.Progress = ResourceManager_->GetResourceAmount( EResourceType::Progress );
	}
	return Snap;
}

FLogBuildMapSnapshot USessionLoggerSubsystem::CaptureBuildMapState( int32 WaveNumber ) const
{
	FLogBuildMapSnapshot Snapshot;
	Snapshot.WaveNumber = WaveNumber;

	if ( !GridManager_.IsValid() )
	{
		return Snapshot;
	}

	AGridManager* Grid = GridManager_.Get();
	Snapshot.Height = Grid->GetGridHeight();
	Snapshot.Width = Grid->GetGridWidth();

	for ( int32 y = 0; y < Snapshot.Height; ++y )
	{
		const int32 RowWidth = Grid->GetRowWidth( y );
		for ( int32 x = 0; x < RowWidth; ++x )
		{
			const FGridCell* Cell = Grid->GetCell( x, y );
			if ( !Cell )
			{
				continue;
			}

			FLogCellState CellState;
			CellState.Coords = FIntPoint( x, y );
			CellState.bIsOccupied = Cell->bIsOccupied;
			CellState.bIsBuildable = Cell->bIsBuildable;
			CellState.bIsWalkable = Cell->bIsWalkable;
			CellState.BuildBonus = Cell->BuildBonus;

			if ( Cell->bIsOccupied && Cell->Occupant.IsValid() )
			{
				ABuilding* Bldg = Cell->Occupant.Get();
				CellState.OccupantClass = Bldg->GetClass()->GetFName();

				// Collect active bonuses
				UBuildingBonusComponent* BonusComp = Bldg->FindComponentByClass<UBuildingBonusComponent>();
				if ( BonusComp )
				{
					const TArray<FBonusApplication>& Apps = BonusComp->GetActiveApplications();
					const TArray<FBuildingBonusEntry>& Entries = BonusComp->GetBonusEntries();

					for ( const FBonusApplication& App : Apps )
					{
						if ( App.EntryIndex_ >= 0 && App.EntryIndex_ < Entries.Num() )
						{
							const FBuildingBonusEntry& Entry = Entries[App.EntryIndex_];
							FLogBonusRecord Rec;
							Rec.SourceBuildingClass = Entry.SourceBuildingClass
							                              ? Entry.SourceBuildingClass->GetFName()
							                              : NAME_None;
							Rec.TargetBuildingClass = CellState.OccupantClass;
							Rec.TargetCell = CellState.Coords;
							Rec.Value = App.AppliedValue_;

							switch ( Entry.Category )
							{
							case EBonusCategory::Production:
								Rec.Category = TEXT( "Production" );
								Rec.ResourceOrStat = UEnum::GetValueAsString( Entry.ResourceType );
								break;
							case EBonusCategory::Maintenance:
								Rec.Category = TEXT( "Maintenance" );
								Rec.ResourceOrStat = UEnum::GetValueAsString( Entry.ResourceType );
								break;
							case EBonusCategory::Stats:
								Rec.Category = TEXT( "Stats" );
								Rec.ResourceOrStat = UEnum::GetValueAsString( Entry.StatType );
								break;
							}

							CellState.ActiveBonuses.Add( Rec );
						}
					}
				}
			}

			Snapshot.Cells.Add( CellState );
		}
	}

	return Snapshot;
}

TArray<FLogBuildingCharacteristics> USessionLoggerSubsystem::CaptureAllBuildingStats() const
{
	TArray<FLogBuildingCharacteristics> Result;

	for ( TActorIterator<ABuilding> It( GetWorld() ); It; ++It )
	{
		ABuilding* Bldg = *It;
		if ( !Bldg || Bldg->IsRuined() )
		{
			continue;
		}

		FLogBuildingCharacteristics Chars;
		Chars.BuildingClass = Bldg->GetClass()->GetFName();
		Chars.Category = GetBuildingCategory( Bldg );

		if ( GridManager_.IsValid() )
		{
			Chars.CellCoords = GridManager_->GetClosestCellCoords( Bldg->GetActorLocation() );
		}

		// Get stats (with cards but INCLUDING adjacency)
		FEntityStats Stats = Bldg->Stats();

		// Subtract adjacency bonuses to get stats with cards only
		UBuildingBonusComponent* BonusComp = Bldg->FindComponentByClass<UBuildingBonusComponent>();
		if ( BonusComp )
		{
			const TArray<FBonusApplication>& Apps = BonusComp->GetActiveApplications();
			const TArray<FBuildingBonusEntry>& Entries = BonusComp->GetBonusEntries();

			for ( const FBonusApplication& App : Apps )
			{
				if ( App.EntryIndex_ < 0 || App.EntryIndex_ >= Entries.Num() )
				{
					continue;
				}
				const FBuildingBonusEntry& Entry = Entries[App.EntryIndex_];

				if ( Entry.Category == EBonusCategory::Stats )
				{
					Stats.AddStat( Entry.StatType, -App.AppliedValue_ );
				}
			}
		}

		Chars.MaxHealth = Stats.MaxHealth();
		Chars.AttackDamage = Stats.AttackDamage();
		Chars.AttackRange = Stats.AttackRange();
		Chars.AttackCooldown = Stats.AttackCooldown();
		Chars.MaxSpeed = Stats.MaxSpeed();
		Chars.SplashRadius = Stats.SplashRadius();
		Chars.BurstCount = Stats.BurstCount();
		Chars.BurstDelay = Stats.BurstDelay();

		// Maintenance and building cost
		FResourceProduction Maint = Bldg->GetMaintenanceCost();
		// Subtract adjacency maintenance bonuses
		if ( BonusComp )
		{
			const TArray<FBonusApplication>& Apps = BonusComp->GetActiveApplications();
			const TArray<FBuildingBonusEntry>& Entries = BonusComp->GetBonusEntries();
			for ( const FBonusApplication& App : Apps )
			{
				if ( App.EntryIndex_ >= 0 && App.EntryIndex_ < Entries.Num() )
				{
					const FBuildingBonusEntry& Entry = Entries[App.EntryIndex_];
					if ( Entry.Category == EBonusCategory::Maintenance )
					{
						Maint.ModifyByType( Entry.ResourceType, static_cast<int32>( -App.AppliedValue_ ) );
					}
				}
			}
		}
		Chars.MaintenanceCost = FLogResourceSnapshot::FromProduction( Maint );
		Chars.BuildingCost = FLogResourceSnapshot::FromProduction( Bldg->GetBuildingCost() );

		// Production (resource buildings only)
		if ( AResourceBuilding* ResBldg = Cast<AResourceBuilding>( Bldg ) )
		{
			FResourceProduction Prod = ResBldg->GetProductionConfig();
			// Subtract adjacency production bonuses
			if ( BonusComp )
			{
				const TArray<FBonusApplication>& Apps = BonusComp->GetActiveApplications();
				const TArray<FBuildingBonusEntry>& Entries = BonusComp->GetBonusEntries();
				for ( const FBonusApplication& App : Apps )
				{
					if ( App.EntryIndex_ >= 0 && App.EntryIndex_ < Entries.Num() )
					{
						const FBuildingBonusEntry& Entry = Entries[App.EntryIndex_];
						if ( Entry.Category == EBonusCategory::Production )
						{
							Prod.ModifyByType( Entry.ResourceType, static_cast<int32>( -App.AppliedValue_ ) );
						}
					}
				}
			}
			Chars.Production = FLogResourceSnapshot::FromProduction( Prod );
		}

		Result.Add( Chars );
	}

	return Result;
}

TArray<FLogBuildingCharacteristics> USessionLoggerSubsystem::CaptureDefensiveBuildingStats() const
{
	TArray<FLogBuildingCharacteristics> All = CaptureAllBuildingStats();
	TArray<FLogBuildingCharacteristics> DefOnly;
	for ( const FLogBuildingCharacteristics& C : All )
	{
		if ( C.Category == TEXT( "Defensive" ) )
		{
			DefOnly.Add( C );
		}
	}
	return DefOnly;
}

void USessionLoggerSubsystem::CaptureEnemySpawnData( int32 WaveIndex )
{
	if ( !CurrentWaveData_ || !WaveManager_.IsValid() )
	{
		return;
	}

	const TArray<FWave>& Waves = WaveManager_->Waves;
	if ( !Waves.IsValidIndex( WaveIndex ) )
	{
		return;
	}

	const FWave& Wave = Waves[WaveIndex];
	for ( int32 i = 0; i < Wave.EnemyGroups.Num(); ++i )
	{
		const FEnemyGroup& Group = Wave.EnemyGroups[i];
		if ( !Group.IsValid() )
		{
			continue;
		}

		FLogEnemySpawnRecord Record;
		Record.EnemyClass = Group.EnemyClass ? Group.EnemyClass->GetFName() : NAME_None;
		Record.Count = Group.Count;
		Record.SpawnPointId = Wave.GetSpawnPointIdForGroup( i );
		Record.GroupIndex = i;

		CurrentWaveData_->EnemiesSpawned.Add( Record );

		// Capture enemy stats from CDO + buffs
		if ( Group.EnemyClass )
		{
			const AUnit* CDO = Group.EnemyClass->GetDefaultObject<AUnit>();
			if ( CDO )
			{
				// Check if we already have stats for this class
				bool bAlreadyHas = false;
				for ( const FLogEnemyTypeStats& Existing : CurrentWaveData_->EnemyTypeStats )
				{
					if ( Existing.EnemyClass == Record.EnemyClass )
					{
						bAlreadyHas = true;
						break;
					}
				}

				if ( !bAlreadyHas )
				{
					FLogEnemyTypeStats EStats;
					EStats.EnemyClass = Record.EnemyClass;

					// Base stats from CDO
					const FEntityStats& Stats = const_cast<AUnit*>( CDO )->Stats();
					EStats.MaxHealth = Stats.MaxHealth();
					EStats.AttackDamage = Stats.AttackDamage();
					EStats.AttackRange = Stats.AttackRange();
					EStats.AttackCooldown = Stats.AttackCooldown();
					EStats.MaxSpeed = Stats.MaxSpeed();
					EStats.SplashRadius = Stats.SplashRadius();
					EStats.BurstCount = Stats.BurstCount();

					// Apply buffs if present
					const FEnemyBuff* Buff = WaveManager_->EnemyBuffs.Find( Group.EnemyClass );
					if ( Buff )
					{
						EStats.MaxHealth =
						    static_cast<int32>( EStats.MaxHealth * Buff->HealthMultiplier );
						EStats.AttackDamage =
						    static_cast<int32>( EStats.AttackDamage * Buff->AttackDamageMultiplier );
						EStats.AttackRange *= Buff->AttackRangeMultiplier;
						EStats.AttackCooldown *= Buff->AttackCooldownMultiplier;
						EStats.MaxSpeed *= Buff->MaxSpeedMultiplier;
					}

					CurrentWaveData_->EnemyTypeStats.Add( EStats );
				}
			}
		}
	}
}

void USessionLoggerSubsystem::CollectBonusDataForBuilding(
    ABuilding* Bldg, const FIntPoint& BldgCoords, TArray<FLogBonusRecord>& OutReceived,
    TArray<FLogBonusRecord>& OutGiven
)
{
	if ( !Bldg )
	{
		return;
	}

	UBuildingBonusComponent* BonusComp = Bldg->FindComponentByClass<UBuildingBonusComponent>();
	if ( !BonusComp )
	{
		return;
	}

	const TArray<FBonusApplication>& Apps = BonusComp->GetActiveApplications();
	const TArray<FBuildingBonusEntry>& Entries = BonusComp->GetBonusEntries();

	for ( const FBonusApplication& App : Apps )
	{
		if ( App.EntryIndex_ < 0 || App.EntryIndex_ >= Entries.Num() )
		{
			continue;
		}

		const FBuildingBonusEntry& Entry = Entries[App.EntryIndex_];

		FLogBonusRecord Rec;
		Rec.SourceBuildingClass = Bldg->GetClass()->GetFName();
		Rec.SourceCell = BldgCoords;
		Rec.Value = App.AppliedValue_;

		if ( App.TargetBuilding_.IsValid() )
		{
			Rec.TargetBuildingClass = App.TargetBuilding_->GetClass()->GetFName();
			if ( GridManager_.IsValid() )
			{
				Rec.TargetCell = GridManager_->GetClosestCellCoords( App.TargetBuilding_->GetActorLocation() );
			}
		}

		switch ( Entry.Category )
		{
		case EBonusCategory::Production:
			Rec.Category = TEXT( "Production" );
			Rec.ResourceOrStat = UEnum::GetValueAsString( Entry.ResourceType );
			break;
		case EBonusCategory::Maintenance:
			Rec.Category = TEXT( "Maintenance" );
			Rec.ResourceOrStat = UEnum::GetValueAsString( Entry.ResourceType );
			break;
		case EBonusCategory::Stats:
			Rec.Category = TEXT( "Stats" );
			Rec.ResourceOrStat = UEnum::GetValueAsString( Entry.StatType );
			break;
		}

		// Determine if this building receives or gives the bonus
		if ( App.TargetBuilding_.Get() == Bldg )
		{
			OutReceived.Add( Rec );
		}
		else
		{
			OutGiven.Add( Rec );
		}
	}
}

void USessionLoggerSubsystem::CaptureBuildingHealthSnapshot()
{
	BuildingHealthAtCombatStart_.Reset();
	for ( TActorIterator<ABuilding> It( GetWorld() ); It; ++It )
	{
		ABuilding* Bldg = *It;
		if ( Bldg && !Bldg->IsRuined() )
		{
			BuildingHealthAtCombatStart_.Add( Bldg, Bldg->Stats().Health() );
		}
	}
}

FLogCombatSummary USessionLoggerSubsystem::BuildCombatSummary()
{
	FLogCombatSummary Summary;
	Summary.WaveDurationSeconds = static_cast<float>( FPlatformTime::Seconds() - CombatStartTime_ );

	// Main base HP
	for ( TActorIterator<AMainBase> It( GetWorld() ); It; ++It )
	{
		AMainBase* Base = *It;
		if ( Base )
		{
			const int32* StartHP = BuildingHealthAtCombatStart_.Find( Base );
			Summary.MainBaseHealthStart = StartHP ? *StartHP : Base->Stats().MaxHealth();
			Summary.MainBaseHealthEnd = FMath::Max( 0, Base->Stats().Health() );
			Summary.MainBaseDamageReceived = static_cast<float>( Summary.MainBaseHealthStart - Summary.MainBaseHealthEnd );
			break;
		}
	}

	// Per-building damage
	for ( const auto& Pair : BuildingHealthAtCombatStart_ )
	{
		ABuilding* Bldg = Pair.Key.Get();
		if ( !Bldg )
		{
			continue;
		}

		const int32 HPBefore = Pair.Value;
		const int32 HPAfter = Bldg->IsRuined() ? 0 : Bldg->Stats().Health();
		const float DmgReceived = static_cast<float>( HPBefore - HPAfter );

		if ( DmgReceived > 0.0f )
		{
			FLogBuildingDamageRecord DmgRec;
			DmgRec.BuildingClass = Bldg->GetClass()->GetFName();
			DmgRec.HealthBefore = HPBefore;
			DmgRec.HealthAfter = HPAfter;
			DmgRec.DamageReceived = DmgReceived;
			if ( GridManager_.IsValid() )
			{
				DmgRec.CellCoords = GridManager_->GetClosestCellCoords( Bldg->GetActorLocation() );
			}
			Summary.BuildingsDamaged.Add( DmgRec );
		}
	}

	return Summary;
}

FLogWaveMetrics USessionLoggerSubsystem::CalculateWaveMetrics() const
{
	FLogWaveMetrics Metrics;

	if ( !CurrentWaveData_ )
	{
		return Metrics;
	}

	// Enemy metrics
	Metrics.EnemyDiversityIndex = CurrentWaveData_->EnemyTypeStats.Num();
	for ( const FLogEnemyTypeStats& ETS : CurrentWaveData_->EnemyTypeStats )
	{
		Metrics.TotalEnemyHP += ETS.TotalCount * ETS.MaxHealth;
		if ( ETS.AttackCooldown > 0.0f )
		{
			Metrics.TotalEnemyDPS += ETS.TotalCount * ( static_cast<float>( ETS.AttackDamage ) / ETS.AttackCooldown );
		}
	}

	// Tower DPS from last build turn's defensive stats
	for ( int32 i = CurrentWaveData_->Turns.Num() - 1; i >= 0; --i )
	{
		const FLogTurnData& Turn = CurrentWaveData_->Turns[i];
		if ( Turn.TurnType == TEXT( "Build" ) && Turn.Defensive.AllDefensiveStats.Num() > 0 )
		{
			for ( const FLogBuildingCharacteristics& DefStat : Turn.Defensive.AllDefensiveStats )
			{
				if ( DefStat.AttackCooldown > 0.0f )
				{
					Metrics.TotalTowerDPS += static_cast<float>( DefStat.AttackDamage ) / DefStat.AttackCooldown;
				}
			}
			break;
		}
	}

	// Ratios
	if ( Metrics.TotalEnemyHP > 0 )
	{
		Metrics.DpsToHpRatio = Metrics.TotalTowerDPS / static_cast<float>( Metrics.TotalEnemyHP );
	}

	// Difficulty score (1-10 scale based on enemy HP/DPS vs tower DPS)
	if ( Metrics.TotalTowerDPS > 0.0f )
	{
		const float Ratio = Metrics.TotalEnemyDPS / Metrics.TotalTowerDPS;
		Metrics.DifficultyScore = FMath::Clamp( Ratio * 5.0f, 1.0f, 10.0f );
	}
	else
	{
		Metrics.DifficultyScore = 10.0f;
	}

	return Metrics;
}

void USessionLoggerSubsystem::CalculateSessionMetrics()
{
	SessionData_.TotalSessionDurationSeconds = static_cast<float>( FPlatformTime::Seconds() - SessionStartTime_ );
	SessionData_.ClosestCallWave = ClosestCallWave_;

	// Sum enemies killed across waves
	int32 TotalKilled = 0;
	for ( const FLogWaveData& Wave : SessionData_.Waves )
	{
		for ( const FLogEnemyTypeStats& ETS : Wave.EnemyTypeStats )
		{
			TotalKilled += ETS.Killed;
		}
	}
	SessionData_.TotalEnemiesKilled = TotalKilled;
}

FString USessionLoggerSubsystem::GetBuildingCategory( const ABuilding* Building ) const
{
	if ( Cast<AMainBase>( Building ) )
	{
		return TEXT( "MainBase" );
	}
	if ( Cast<ADefensiveBuilding>( Building ) )
	{
		return TEXT( "Defensive" );
	}
	if ( Cast<AResourceBuilding>( Building ) )
	{
		return TEXT( "Resource" );
	}
	if ( Cast<AAdditiveBuilding>( Building ) )
	{
		return TEXT( "Additive" );
	}
	return TEXT( "Other" );
}

// ============================================================================
// Wave/Turn Lifecycle
// ============================================================================

void USessionLoggerSubsystem::BeginNewWave( int32 WaveNumber )
{
	FLogWaveData NewWave;
	NewWave.WaveNumber = WaveNumber;
	NewWave.Turns.Reserve( 3 );
	SessionData_.Waves.Add( NewWave );
	CurrentWaveData_ = &SessionData_.Waves.Last();
}

void USessionLoggerSubsystem::BeginNewTurn( int32 TurnNumber, const FString& TurnType )
{
	if ( !CurrentWaveData_ )
	{
		return;
	}

	FLogTurnData NewTurn;
	NewTurn.TurnNumber = TurnNumber;
	NewTurn.TurnType = TurnType;

	// Timing
	TurnStartTime_ = FPlatformTime::Seconds();

	// Capture resources at turn start
	ResourcesAtTurnStart_ = CaptureCurrentResources();
	NewTurn.ResourcesAtStart = ResourcesAtTurnStart_;

	// Reset per-turn accumulators
	AccumulatedBuildCost_ = FLogResourceSnapshot();
	AccumulatedDefensiveBuildCost_ = FLogResourceSnapshot();

	CurrentWaveData_->Turns.Add( NewTurn );
	CurrentTurnData_ = &CurrentWaveData_->Turns.Last();
}

void USessionLoggerSubsystem::EndCurrentTurn()
{
	if ( !CurrentTurnData_ )
	{
		return;
	}

	// Turn duration
	CurrentTurnData_->TurnDurationSeconds = static_cast<float>( FPlatformTime::Seconds() - TurnStartTime_ );

	// Set accumulated build cost
	CurrentTurnData_->SpentOnBuilding = AccumulatedBuildCost_;
	SessionData_.TotalSpentOnBuilding += AccumulatedBuildCost_;

	// Resources at end of turn
	CurrentTurnData_->ResourcesAtEnd = CaptureCurrentResources();

	// Calculate total maintenance paid this turn (all buildings)
	FLogResourceSnapshot TotalMaint;
	for ( TActorIterator<ABuilding> MaintIt( GetWorld() ); MaintIt; ++MaintIt )
	{
		ABuilding* Bldg = *MaintIt;
		if ( Bldg && !Bldg->IsRuined() )
		{
			TotalMaint += FLogResourceSnapshot::FromProduction( Bldg->GetMaintenanceCost() );
		}
	}
	// Apply card-based maintenance reductions
	if ( UCardSubsystem* CardSub = UCardSubsystem::Get( this ) )
	{
		const FEconomyBonuses& Bonuses = CardSub->GetEconomyBonuses();
		TotalMaint.Gold = FMath::Max( 0, TotalMaint.Gold - Bonuses.GoldMaintenanceReduction );
		TotalMaint.Food = FMath::Max( 0, TotalMaint.Food - Bonuses.FoodMaintenanceReduction );
		TotalMaint.Population = FMath::Max( 0, TotalMaint.Population - Bonuses.PopulationMaintenanceReduction );
		TotalMaint.Progress = FMath::Max( 0, TotalMaint.Progress - Bonuses.ProgressMaintenanceReduction );
	}
	CurrentTurnData_->MaintenancePaid = TotalMaint;

	// Calculate adjacency bonus income (production bonuses from BonusComponents on resource buildings)
	FLogResourceSnapshot AdjBonusIncome;
	for ( TActorIterator<AResourceBuilding> ResIt( GetWorld() ); ResIt; ++ResIt )
	{
		AResourceBuilding* ResBldg = *ResIt;
		if ( !ResBldg || ResBldg->IsRuined() )
		{
			continue;
		}

		UBuildingBonusComponent* BonusComp = ResBldg->FindComponentByClass<UBuildingBonusComponent>();
		if ( !BonusComp )
		{
			continue;
		}

		const TArray<FBonusApplication>& Apps = BonusComp->GetActiveApplications();
		const TArray<FBuildingBonusEntry>& Entries = BonusComp->GetBonusEntries();
		for ( const FBonusApplication& App : Apps )
		{
			if ( App.EntryIndex_ < 0 || App.EntryIndex_ >= Entries.Num() )
			{
				continue;
			}
			if ( App.TargetBuilding_.Get() != ResBldg )
			{
				continue;
			}
			const FBuildingBonusEntry& Entry = Entries[App.EntryIndex_];
			if ( Entry.Category == EBonusCategory::Production )
			{
				FResourceProduction BonusProd;
				BonusProd.ModifyByType( Entry.ResourceType, static_cast<int32>( App.AppliedValue_ ) );
				AdjBonusIncome += FLogResourceSnapshot::FromProduction( BonusProd );
			}
		}
	}
	CurrentTurnData_->AdjacencyBonusIncome = AdjBonusIncome;

	// Calculate income from transition (derived from resource balance equation)
	// Income = ResourcesAtEnd - ResourcesAtStart + SpentOnBuilding + MaintenancePaid - AdjacencyBonusIncome
	FLogResourceSnapshot ComputedIncome = CurrentTurnData_->ResourcesAtEnd
	    - CurrentTurnData_->ResourcesAtStart
	    + CurrentTurnData_->SpentOnBuilding
	    + CurrentTurnData_->MaintenancePaid
	    - CurrentTurnData_->AdjacencyBonusIncome;
	// Clamp negatives to 0 (can happen in combat turns with no income)
	ComputedIncome.Gold = FMath::Max( 0, ComputedIncome.Gold );
	ComputedIncome.Food = FMath::Max( 0, ComputedIncome.Food );
	ComputedIncome.Population = FMath::Max( 0, ComputedIncome.Population );
	ComputedIncome.Progress = FMath::Max( 0, ComputedIncome.Progress );
	CurrentTurnData_->IncomeFromTransition = ComputedIncome;

	// Capture building characteristics
	CurrentTurnData_->AllBuildingCharacteristics = CaptureAllBuildingStats();

	// Defensive data
	CurrentTurnData_->Defensive.SpentOnDefensive = AccumulatedDefensiveBuildCost_;

	// Calculate defensive maintenance
	FLogResourceSnapshot DefMaint;
	for ( TActorIterator<ADefensiveBuilding> It( GetWorld() ); It; ++It )
	{
		ADefensiveBuilding* Def = *It;
		if ( Def && !Def->IsRuined() )
		{
			DefMaint += FLogResourceSnapshot::FromProduction( Def->GetMaintenanceCost() );
		}
	}
	CurrentTurnData_->Defensive.DefensiveMaintenance = DefMaint;

	// Defensive building stats (without adjacency)
	CurrentTurnData_->Defensive.AllDefensiveStats = CaptureDefensiveBuildingStats();

	// Collect adjacency bonuses for defensive buildings
	for ( TActorIterator<ADefensiveBuilding> It( GetWorld() ); It; ++It )
	{
		ADefensiveBuilding* Def = *It;
		if ( !Def || Def->IsRuined() )
		{
			continue;
		}

		UBuildingBonusComponent* BonusComp = Def->FindComponentByClass<UBuildingBonusComponent>();
		if ( !BonusComp )
		{
			continue;
		}

		const TArray<FBonusApplication>& Apps = BonusComp->GetActiveApplications();
		const TArray<FBuildingBonusEntry>& Entries = BonusComp->GetBonusEntries();
		for ( const FBonusApplication& App : Apps )
		{
			if ( App.EntryIndex_ >= 0 && App.EntryIndex_ < Entries.Num() && App.TargetBuilding_.Get() == Def )
			{
				const FBuildingBonusEntry& Entry = Entries[App.EntryIndex_];
				FLogBonusRecord Rec;
				Rec.SourceBuildingClass = Entry.SourceBuildingClass ? Entry.SourceBuildingClass->GetFName() : NAME_None;
				Rec.TargetBuildingClass = Def->GetClass()->GetFName();
				Rec.Value = App.AppliedValue_;

				if ( GridManager_.IsValid() )
				{
					Rec.TargetCell = GridManager_->GetClosestCellCoords( Def->GetActorLocation() );
				}

				switch ( Entry.Category )
				{
				case EBonusCategory::Production:
					Rec.Category = TEXT( "Production" );
					Rec.ResourceOrStat = UEnum::GetValueAsString( Entry.ResourceType );
					break;
				case EBonusCategory::Maintenance:
					Rec.Category = TEXT( "Maintenance" );
					Rec.ResourceOrStat = UEnum::GetValueAsString( Entry.ResourceType );
					break;
				case EBonusCategory::Stats:
					Rec.Category = TEXT( "Stats" );
					Rec.ResourceOrStat = UEnum::GetValueAsString( Entry.StatType );
					break;
				}

				CurrentTurnData_->Defensive.AdjacencyBonuses.Add( Rec );
			}
		}
	}

	// Tower damage (only in combat turn)
	if ( CurrentTurnData_->TurnType == TEXT( "Combat" ) )
	{
		for ( const auto& Pair : TowerDamageMap_ )
		{
			FLogTowerDamageRecord DmgRecord;
			DmgRecord.TowerClass = Pair.Value.InstigatorClass;
			DmgRecord.AttackType = Pair.Value.AttackType;
			DmgRecord.DirectDamage = Pair.Value.DirectDamage;
			DmgRecord.SplashDamage = Pair.Value.SplashDamage;
			DmgRecord.TotalDamage = Pair.Value.DirectDamage + Pair.Value.SplashDamage;
			DmgRecord.KillCount = Pair.Value.KillCount;
			DmgRecord.ShotsTotal = Pair.Value.ShotsTotal;
			DmgRecord.ShotsHit = Pair.Value.ShotsHit;
			CurrentTurnData_->Defensive.TowerDamage.Add( DmgRecord );
		}

		// Combat summary (building damage, base HP, wave duration)
		CurrentTurnData_->Defensive.CombatSummary = BuildCombatSummary();

		// Track closest-call wave (lowest base HP)
		const int32 BaseHPAfter = CurrentTurnData_->Defensive.CombatSummary.MainBaseHealthEnd;
		if ( BaseHPAfter < LowestBaseHP_ )
		{
			LowestBaseHP_ = BaseHPAfter;
			ClosestCallWave_ = CurrentWaveNumber_;
		}
	}

	CurrentTurnData_ = nullptr;
}

void USessionLoggerSubsystem::FinalizeWave()
{
	if ( !CurrentWaveData_ )
	{
		return;
	}

	// Aggregate spawn counts and spawn points per enemy type
	for ( FLogEnemyTypeStats& ETS : CurrentWaveData_->EnemyTypeStats )
	{
		TSet<FName> UniqueSpawnPoints;
		for ( const FLogEnemySpawnRecord& Spawn : CurrentWaveData_->EnemiesSpawned )
		{
			if ( Spawn.EnemyClass == ETS.EnemyClass )
			{
				ETS.TotalCount += Spawn.Count;
				if ( !Spawn.SpawnPointId.IsNone() )
				{
					UniqueSpawnPoints.Add( Spawn.SpawnPointId );
				}
			}
		}
		ETS.SpawnPoints = UniqueSpawnPoints.Array();
	}

	// Fill enemy damage and penetration stats from accumulators
	for ( FLogEnemyTypeStats& ETS : CurrentWaveData_->EnemyTypeStats )
	{
		FLogEnemyDamageAccumulator* Acc = EnemyDamageMap_.Find( ETS.EnemyClass );
		if ( Acc )
		{
			ETS.DamageToDefensive = Acc->DamageToDefensive;
			ETS.DamageToEconomic = Acc->DamageToEconomic;
			ETS.DamageToWalls = Acc->DamageToWalls;
			ETS.TotalDamage = Acc->TotalDamage;
			ETS.BuildingsDestroyedCounts = Acc->BuildingsDestroyedCounts;

			// Penetration stats
			ETS.Killed = Acc->Killed;
			ETS.ReachedBase = ETS.TotalCount - Acc->Killed; // Enemies not killed = reached base
			if ( Acc->PathProgressValues.Num() > 0 )
			{
				float Sum = 0.0f;
				float Max = 0.0f;
				for ( float P : Acc->PathProgressValues )
				{
					Sum += P;
					Max = FMath::Max( Max, P );
				}
				ETS.AvgPathProgress = Sum / Acc->PathProgressValues.Num();
				ETS.MaxPathProgress = Max;
			}
		}
	}

	// Calculate wave metrics
	CurrentWaveData_->WaveMetrics = CalculateWaveMetrics();

	// Capture build map at end of wave
	CurrentWaveData_->BuildMap = CaptureBuildMapState( CurrentWaveData_->WaveNumber );

	// Calculate totals for income/maintenance
	FLogResourceSnapshot WaveIncome;
	FLogResourceSnapshot WaveMaintenance;
	for ( const FLogTurnData& Turn : CurrentWaveData_->Turns )
	{
		WaveIncome += Turn.IncomeFromTransition;
		WaveMaintenance += Turn.MaintenancePaid;
	}
	SessionData_.TotalIncome += WaveIncome;
	SessionData_.TotalMaintenancePaid += WaveMaintenance;

	// Broadcast wave finalization
	OnWaveDataFinalized.Broadcast( *CurrentWaveData_ );

	CurrentWaveData_ = nullptr;
}

void USessionLoggerSubsystem::FinalizeSession( bool bVictory )
{
	if ( !bIsLogging_ )
	{
		return;
	}

	// If combat turn is still active, end it
	if ( CurrentTurnData_ )
	{
		EndCurrentTurn();
	}

	// If wave is still active, finalize it
	if ( CurrentWaveData_ )
	{
		FinalizeWave();
	}

	SessionData_.bVictory = bVictory;
	SessionData_.WavesSurvived = CurrentWaveNumber_;

	// Capture final field state
	SessionData_.FinalFieldState = CaptureBuildMapState( CurrentWaveNumber_ );

	// Calculate session-level analytics
	CalculateSessionMetrics();

	// Let collectors append their data
	for ( ISessionDataCollector* Collector : DataCollectors_ )
	{
		Collector->OnGameEnded( bVictory );
	}

	// Write JSON
	WriteSessionToFile();

	// Broadcast session finalized
	OnSessionFinalized.Broadcast( SessionData_ );

	bIsLogging_ = false;
}

// ============================================================================
// JSON Output
// ============================================================================

void USessionLoggerSubsystem::WriteSessionToFile()
{
	TSharedPtr<FJsonObject> RootJson = SessionData_.ToJson();

	// Let collectors append
	for ( ISessionDataCollector* Collector : DataCollectors_ )
	{
		Collector->AppendToJson( RootJson );
	}

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create( &JsonString );
	FJsonSerializer::Serialize( RootJson.ToSharedRef(), Writer );

	const FString FilePath = GetOutputFilePath();

	// Ensure directory exists
	const FString Dir = FPaths::GetPath( FilePath );
	IPlatformFile::GetPlatformPhysical().CreateDirectoryTree( *Dir );

	// Async write to avoid blocking game thread
	AsyncTask( ENamedThreads::AnyBackgroundThreadNormalTask, [JsonString, FilePath]()
	{
		if ( FFileHelper::SaveStringToFile( JsonString, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM ) )
		{
			UE_LOG( LogSessionLogger, Log, TEXT( "Session log saved to: %s" ), *FilePath );
		}
		else
		{
			UE_LOG( LogSessionLogger, Error, TEXT( "Failed to save session log to: %s" ), *FilePath );
		}
	} );
}

FString USessionLoggerSubsystem::GetOutputFilePath() const
{
	FString MapName = SessionData_.MapName;
	const FString Outcome = SessionData_.bVictory ? TEXT( "Victory" ) : TEXT( "Defeat" );
	const FString Timestamp = FDateTime::Now().ToString( TEXT( "%Y-%m-%d_%H-%M-%S" ) );
	const FString FileName = FString::Printf( TEXT( "%s_%s_%s.json" ), *MapName, *Outcome, *Timestamp );
	return FPaths::ProjectSavedDir() / TEXT( "SessionLogs" ) / FileName;
}
