#include "Lords_Frontiers/Public/Match/MatchStatsTracker.h"

#include "Building/Building.h"
#include "Building/Construction/BuildManager.h"
#include "Building/DefensiveBuilding.h"
#include "Core/CoreManager.h"
#include "Core/GameLoop/GameLoopManager.h"
#include "Core/Subsystems/SessionLogger/DamageEvent.h"
#include "Engine/GameInstance.h"
#include "EntityStats.h"
#include "Kismet/GameplayStatics.h"
#include "Lords_Frontiers/Public/Match/LeaderboardSubsystem.h"
#include "Lords_Frontiers/Public/Resources/ResourceManager.h"
#include "Lords_Frontiers/Public/Units/UnitEvents.h"

void UMatchStatsTracker::Deinitialize()
{
	UnsubscribeFromGameEvents();
	Super::Deinitialize();
}

void UMatchStatsTracker::BeginMatch( UMatchScoringConfig* config )
{
	Config = config;
	Reset();

	UE_LOG( LogTemp, Log, TEXT( "MatchStatsTracker::BeginMatch config=%s" ), Config ? *Config->GetName() : TEXT( "<null>" ) );

	if ( Config )
	{
		if ( UGameInstance* gi = GetGameInstance() )
		{
			if ( ULeaderboardSubsystem* board = gi->GetSubsystem<ULeaderboardSubsystem>() )
			{
				if ( !Config->LeaderboardSlotName.IsEmpty() )
				{
					board->Load( Config->LeaderboardSlotName );
				}
				board->LoadSeedsFromConfig( Config->LeaderboardConfig );
				UE_LOG( LogTemp, Log, TEXT( "MatchStatsTracker: seeds loaded = %d (config=%s)" ),
					board->GetSeedCount(),
					Config->LeaderboardConfig ? *Config->LeaderboardConfig->GetName() : TEXT( "<null>" ) );
			}
			else
			{
				UE_LOG( LogTemp, Warning, TEXT( "MatchStatsTracker: ULeaderboardSubsystem не найден." ) );
			}
		}
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT( "MatchStatsTracker::BeginMatch: config == nullptr — пресет-записи не загружены и счёт будет 0." ) );
	}

	UnsubscribeFromGameEvents();
	SubscribeToGameEvents();
}

void UMatchStatsTracker::SubscribeToGameEvents()
{
	UnitDiedHandle = FUnitEvents::OnUnitDied.AddUObject( this, &UMatchStatsTracker::HandleUnitDied );
	DamageDealtHandle = FDamageEvents::OnDamageDealt.AddUObject( this, &UMatchStatsTracker::HandleDamageDealt );

	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( UGameLoopManager* gl = core->GetGameLoop() )
		{
			gl->OnWaveChanged.AddUniqueDynamic( this, &UMatchStatsTracker::HandleWaveChanged );
			BoundGameLoop = gl;
		}
		if ( ABuildManager* bm = core->GetBuildManager() )
		{
			bm->OnBuildingConfirmed.AddUniqueDynamic( this, &UMatchStatsTracker::HandleBuildingConfirmed );
			BoundBuildManager = bm;
		}
		if ( UResourceManager* rm = core->GetResourceManager() )
		{
			rm->OnResourceAdded.AddUniqueDynamic( this, &UMatchStatsTracker::HandleResourceAdded );
			BoundResourceManager = rm;
		}
	}
}

void UMatchStatsTracker::UnsubscribeFromGameEvents()
{
	if ( UnitDiedHandle.IsValid() )
	{
		FUnitEvents::OnUnitDied.Remove( UnitDiedHandle );
		UnitDiedHandle.Reset();
	}
	if ( DamageDealtHandle.IsValid() )
	{
		FDamageEvents::OnDamageDealt.Remove( DamageDealtHandle );
		DamageDealtHandle.Reset();
	}
	if ( UGameLoopManager* gl = BoundGameLoop.Get() )
	{
		gl->OnWaveChanged.RemoveDynamic( this, &UMatchStatsTracker::HandleWaveChanged );
	}
	BoundGameLoop.Reset();
	if ( ABuildManager* bm = BoundBuildManager.Get() )
	{
		bm->OnBuildingConfirmed.RemoveDynamic( this, &UMatchStatsTracker::HandleBuildingConfirmed );
	}
	BoundBuildManager.Reset();
	if ( UResourceManager* rm = BoundResourceManager.Get() )
	{
		rm->OnResourceAdded.RemoveDynamic( this, &UMatchStatsTracker::HandleResourceAdded );
	}
	BoundResourceManager.Reset();
}

void UMatchStatsTracker::HandleResourceAdded( EResourceType type, int32 delta )
{
	OnResourceEarned( type, delta );
}

void UMatchStatsTracker::HandleWaveChanged( int32 currentWave, int32 totalWaves )
{
	if ( currentWave > 0 )
	{
		OnWaveSurvived( currentWave - 1 );
	}
}

void UMatchStatsTracker::HandleBuildingConfirmed( ABuilding* building, FIntPoint cell )
{
	if ( const ADefensiveBuilding* def = Cast<ADefensiveBuilding>( building ) )
	{
		OnTowerBuilt( def->DefensiveTowerType() );
	}
}

void UMatchStatsTracker::HandleUnitDied( AUnit* unit )
{
	if ( !unit )
	{
		return;
	}
	if ( unit->Team() != ETeam::Dog )
	{
		return;
	}
	OnEnemyKilled( unit->GetClass(), unit->IsBossForStats() );
}

void UMatchStatsTracker::HandleDamageDealt( AActor* instigator, AActor* target, int damage, bool bIsSplash )
{
	if ( damage <= 0 || !target )
	{
		return;
	}
	if ( const AUnit* enemy = Cast<AUnit>( target ) )
	{
		if ( enemy->Team() == ETeam::Dog )
		{
			OnDamageDealt( damage );
		}
	}
}

void UMatchStatsTracker::Reset()
{
	Stats = FMatchStats();
	Broadcast();
}

void UMatchStatsTracker::OnWaveSurvived( int32 waveIndex )
{
	Stats.WavesSurvived = FMath::Max( Stats.WavesSurvived, waveIndex + 1 );
	Broadcast();
}

void UMatchStatsTracker::OnEnemyKilled( TSubclassOf<AUnit> enemyClass, bool bIsBoss )
{
	if ( !enemyClass )
	{
		return;
	}

	Stats.EnemiesKilled += 1;
	Stats.EnemiesKilledByClass.FindOrAdd( enemyClass ) += 1;

	if ( bIsBoss )
	{
		Stats.BossesKilled += 1;
		Stats.BossesKilledByClass.FindOrAdd( enemyClass ) += 1;
	}

	Broadcast();
}

void UMatchStatsTracker::OnEnemySurvived( TSubclassOf<AUnit> enemyClass )
{
	Stats.EnemiesSurvived += 1;
	Broadcast();
}

void UMatchStatsTracker::OnDamageDealt( int32 amount )
{
	if ( amount <= 0 )
	{
		return;
	}
	Stats.DamageDealt += amount;
	Broadcast();
}

void UMatchStatsTracker::OnTowerBuilt( EDefensiveTowerType towerType )
{
	Stats.TowersBuilt += 1;
	Stats.TowersBuiltByType.FindOrAdd( towerType ) += 1;
	Broadcast();
}

void UMatchStatsTracker::OnResourceEarned( EResourceType resourceType, int32 amount )
{
	if ( amount <= 0 || resourceType == EResourceType::None || resourceType == EResourceType::Max )
	{
		return;
	}
	Stats.ResourcesEarned.FindOrAdd( resourceType ) += amount;
	Broadcast();
}

FScoreBreakdown UMatchStatsTracker::ComputeBreakdown() const
{
	FScoreBreakdown breakdown;
	if ( !Config )
	{
		return breakdown;
	}

	for ( int32 w = 0; w < Stats.WavesSurvived; ++w )
	{
		breakdown.WavesScore += Config->PointsPerWave;
		breakdown.WavesScore += static_cast<int64>( Config->PointsPerWaveScaling ) * w;
	}

	for ( const TPair<TSubclassOf<AUnit>, int32>& pair : Stats.EnemiesKilledByClass )
	{
		const int32 total = pair.Value;
		if ( total <= 0 )
		{
			continue;
		}
		const int32 bossCount = Stats.BossesKilledByClass.FindRef( pair.Key );
		const int32 regularCount = FMath::Max( 0, total - bossCount );

		if ( const int32* override_ = Config->PointsPerEnemyClass.Find( pair.Key ) )
		{
			breakdown.EnemiesScore += static_cast<int64>( *override_ ) * regularCount;
			breakdown.BossesScore += static_cast<int64>( *override_ ) * bossCount;
		}
		else
		{
			breakdown.EnemiesScore += static_cast<int64>( Config->DefaultPointsPerEnemy ) * regularCount;
			breakdown.BossesScore += static_cast<int64>( Config->DefaultPointsPerBoss ) * bossCount;
		}
	}

	breakdown.DamageScore = static_cast<int64>( Stats.DamageDealt * Config->PointsPerDamage );

	for ( const TPair<EDefensiveTowerType, int32>& pair : Stats.TowersBuiltByType )
	{
		if ( const int32* per = Config->PointsPerTowerType.Find( pair.Key ) )
		{
			breakdown.TowersScore += static_cast<int64>( *per ) * pair.Value;
		}
	}

	for ( const TPair<EResourceType, int64>& pair : Stats.ResourcesEarned )
	{
		if ( const float* per = Config->PointsPerResource.Find( pair.Key ) )
		{
			breakdown.ResourcesScore += static_cast<int64>( pair.Value * ( *per ) );
		}
	}

	breakdown.Total = breakdown.WavesScore + breakdown.EnemiesScore + breakdown.BossesScore +
					  breakdown.DamageScore + breakdown.TowersScore + breakdown.ResourcesScore;
	return breakdown;
}

int64 UMatchStatsTracker::ComputeScore() const
{
	return ComputeBreakdown().Total;
}

int32 UMatchStatsTracker::GetTowersBuiltOfType( EDefensiveTowerType towerType ) const
{
	return Stats.TowersBuiltByType.FindRef( towerType );
}

int64 UMatchStatsTracker::GetResourceEarned( EResourceType resourceType ) const
{
	return Stats.ResourcesEarned.FindRef( resourceType );
}

FLeaderboardEntry UMatchStatsTracker::BuildEntry( const FString& playerName ) const
{
	FString name = playerName;
	if ( name.IsEmpty() )
	{
		name = ( Config && !Config->PlayerEntryName.IsEmpty() ) ? Config->PlayerEntryName : TEXT( "Ты" );
	}

	FLeaderboardEntry entry;
	entry.PlayerName = name;
	entry.Score = ComputeScore();
	entry.Stats = Stats;
	entry.Timestamp = FDateTime::Now();
	entry.bManual = false;
	entry.bIsCurrentPlayer = true;
	return entry;
}

bool UMatchStatsTracker::FinalizeAndPush( const FString& playerName )
{
	UGameInstance* gi = GetGameInstance();
	if ( !gi )
	{
		return false;
	}
	ULeaderboardSubsystem* board = gi->GetSubsystem<ULeaderboardSubsystem>();
	if ( !board )
	{
		return false;
	}

	const FLeaderboardEntry entry = BuildEntry( playerName );
	const int32 maxEntries = Config ? Config->LeaderboardSize : 10;
	return board->AddEntry( entry, maxEntries );
}

void UMatchStatsTracker::Broadcast()
{
	OnScoreChanged.Broadcast( ComputeScore(), Stats );
}
