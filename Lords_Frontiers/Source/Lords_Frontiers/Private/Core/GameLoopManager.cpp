#include "Core/GameLoopManager.h"

#include "Resources/EconomyComponent.h"
#include "Resources/ResourceManager.h"
#include "TimerManager.h"
#include "Waves/WaveManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC( LogGameLoop, Log, All );

UGameLoopManager::UGameLoopManager()
{
}

UGameLoopManager::~UGameLoopManager()
{
	Cleanup();
}

void UGameLoopManager::Initialize(
    UGameLoopConfig* inConfig, AWaveManager* inWaveManager, UResourceManager* inResourceManager,
    UEconomyComponent* inEconomyComponent
)
{
	if ( inConfig )
	{
		Config_ = inConfig;
	}
	else if ( !Config_ )
	{
		Config_ = NewObject<UGameLoopConfig>( this );
		Log( TEXT( "Created default GameLoopConfig" ) );
	}

	WaveManager_ = inWaveManager;
	ResourceManager_ = inResourceManager;
	EconomyComponent_ = inEconomyComponent;

	if ( WaveManager_.IsValid() && !bIsBoundToWaveManager_ )
	{
		BindToWaveManager();
	}

	bIsInitialized_ = ResourceManager_.IsValid();

	Log( FString::Printf(
	    TEXT( "Initialized. Config: %s, WaveManager: %s, ResourceManager: %s" ),
	    Config_ ? TEXT( "OK" ) : TEXT( "MISSING" ), WaveManager_.IsValid() ? TEXT( "OK" ) : TEXT( "MISSING" ),
	    ResourceManager_.IsValid() ? TEXT( "OK" ) : TEXT( "MISSING" )
	) );
}

void UGameLoopManager::SetConfig( UGameLoopConfig* newConfig )
{
	if ( newConfig )
	{
		Config_ = newConfig;
		Log( FString::Printf(
		    TEXT( "Config set: StartGold=%d, BuildTurns=%d, CombatDuration=%.1f" ), Config_->StartingResources.Gold,
		    Config_->BuildTurnsBeforeCombat, Config_->CombatDuration
		) );
	}
}

void UGameLoopManager::UpdateDependencies(
    AWaveManager* inWaveManager, UResourceManager* inResourceManager, UEconomyComponent* inEconomyComponent
)
{
	if ( WaveManager_.IsValid() && WaveManager_.Get() != inWaveManager )
	{
		UnbindFromWaveManager();
	}

	WaveManager_ = inWaveManager;
	ResourceManager_ = inResourceManager;
	EconomyComponent_ = inEconomyComponent;

	if ( WaveManager_.IsValid() && !bIsBoundToWaveManager_ )
	{
		BindToWaveManager();
	}

	Log( FString::Printf(
	    TEXT( "Dependencies updated. WaveManager: %s, ResourceManager: %s" ),
	    WaveManager_.IsValid() ? TEXT( "OK" ) : TEXT( "MISSING" ),
	    ResourceManager_.IsValid() ? TEXT( "OK" ) : TEXT( "MISSING" )
	) );
}

void UGameLoopManager::Cleanup()
{
	Log( TEXT( "Cleanup started" ) );

	bIsGameStarted_ = false;
	bIsPaused_ = false;

	UnbindFromWaveManager();

	if ( UWorld* world = GetWorldSafe() )
	{
		world->GetTimerManager().ClearTimer( CombatStartDelayHandle_ );
		world->GetTimerManager().ClearTimer( BuildingRestoreTimerHandle_ );
	}

	CurrentPhase_ = EGameLoopPhase::None;
	CurrentWave_ = 0;
	CurrentBuildTurn_ = 0;
	CombatTimeRemaining_ = 0.0f;
	bWaveCompleted_ = false;
	bPerfectWave_ = true;
	bIsInitialized_ = false;

	WaveManager_.Reset();
	ResourceManager_.Reset();
	EconomyComponent_.Reset();

	Log( TEXT( "Cleanup completed" ) );
}

bool UGameLoopManager::IsTickable() const
{
	return bIsInitialized_ && bIsGameStarted_ && !bIsPaused_;
}

TStatId UGameLoopManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT( UGameLoopManager, STATGROUP_Tickables );
}

UWorld* UGameLoopManager::GetTickableGameObjectWorld() const
{
	return GetWorldSafe();
}

UWorld* UGameLoopManager::GetWorldSafe() const
{
	if ( UObject* outer = GetOuter() )
	{
		if ( UGameInstance* gi = Cast<UGameInstance>( outer->GetOuter() ) )
		{
			return gi->GetWorld();
		}

		if ( UGameInstance* gi = Cast<UGameInstance>( outer ) )
		{
			return gi->GetWorld();
		}
	}
	return nullptr;
}

void UGameLoopManager::Tick( float deltaTime )
{
	if ( CurrentPhase_ == EGameLoopPhase::Combat )
	{
		UpdateCombatTimer( deltaTime );
	}
}

void UGameLoopManager::StartGame()
{
	if ( !bIsInitialized_ )
	{
		Log( TEXT( "ERROR: Cannot start - not initialized!" ) );
		return;
	}

	if ( bIsGameStarted_ )
	{
		Log( TEXT( "WARNING: Game already started" ) );
		return;
	}

	bIsGameStarted_ = true;
	bIsPaused_ = false;
	CurrentWave_ = 1;
	CurrentBuildTurn_ = 0;
	bPerfectWave_ = true;

	Log( TEXT( "=== GAME STARTED ===" ) );

	OnWaveChanged.Broadcast( CurrentWave_, GetTotalWaves() );

	EnterStartupPhase();
}

void UGameLoopManager::RestartGame()
{
	Log( TEXT( "=== GAME RESTARTING ===" ) );

	UnbindFromWaveManager();

	CurrentPhase_ = EGameLoopPhase::None;
	CurrentWave_ = 0;
	CurrentBuildTurn_ = 0;
	CombatTimeRemaining_ = 0.0f;
	bIsGameStarted_ = false;
	bIsPaused_ = false;
	bWaveCompleted_ = false;
	bPerfectWave_ = true;

	if ( AWaveManager* wm = WaveManager_.Get() )
	{
		wm->CancelCurrentWave();
		wm->RestartWaves();
	}

	if ( UWorld* world = GetWorldSafe() )
	{
		world->GetTimerManager().ClearTimer( CombatStartDelayHandle_ );
	}

	if ( WaveManager_.IsValid() )
	{
		BindToWaveManager();
	}

	StartGame();
}

void UGameLoopManager::PauseGame()
{
	if ( !bIsGameStarted_ || bIsPaused_ )
	{
		return;
	}

	PhaseBeforePause_ = CurrentPhase_;
	bIsPaused_ = true;
	SetPhase( EGameLoopPhase::Paused );

	Log( TEXT( "Game PAUSED" ) );
}

void UGameLoopManager::ResumeGame()
{
	if ( !bIsPaused_ )
	{
		return;
	}

	bIsPaused_ = false;
	SetPhase( PhaseBeforePause_ );

	Log( TEXT( "Game RESUMED" ) );
}

void UGameLoopManager::EndBuildTurn()
{
	if ( !CanEndBuildTurn() )
	{
		Log( TEXT( "WARNING: Cannot end build turn now" ) );
		return;
	}

	Log( FString::Printf( TEXT( "End Build Turn %d" ), CurrentBuildTurn_ ) );

	if ( CurrentBuildTurn_ >= GetMaxBuildTurns() )
	{
		EnterCombatPhase();
	}
	else
	{
		CollectBuildingIncome();
		CurrentBuildTurn_++;
		OnBuildTurnChanged.Broadcast( CurrentBuildTurn_, GetMaxBuildTurns() );
		BroadcastButtonStates();

		Log( FString::Printf( TEXT( "Build Turn %d / %d" ), CurrentBuildTurn_, GetMaxBuildTurns() ) );
	}
}

void UGameLoopManager::StartCombatEarly()
{
	if ( !CanStartCombatEarly() )
	{
		Log( TEXT( "WARNING: Cannot start combat early now" ) );
		return;
	}

	Log( TEXT( "Starting combat EARLY" ) );

	EnterCombatPhase();
}

void UGameLoopManager::ReportDamageTaken( float damageAmount )
{
	if ( damageAmount > 0.0f && bPerfectWave_ )
	{
		bPerfectWave_ = false;
		Log( FString::Printf( TEXT( "Perfect wave lost! Damage: %.1f" ), damageAmount ) );
	}
}

int32 UGameLoopManager::GetTotalWaves() const
{
	if ( Config_ && Config_->WavesToWin > 0 )
	{
		return Config_->WavesToWin;
	}

	if ( AWaveManager* wm = WaveManager_.Get() )
	{
		return wm->Waves.Num();
	}

	return GameLoopDefaults::cDefaultWavesToWin;
}

int32 UGameLoopManager::GetMaxBuildTurns() const
{
	return Config_ ? Config_->BuildTurnsBeforeCombat : GameLoopDefaults::cDefaultBuildTurns;
}

int32 UGameLoopManager::GetRemainingBuildTurns() const
{
	return FMath::Max( 0, GetMaxBuildTurns() - CurrentBuildTurn_ );
}

float UGameLoopManager::GetCombatTotalTime() const
{
	return Config_ ? Config_->CombatDuration : GameLoopDefaults::cDefaultCombatDuration;
}

bool UGameLoopManager::CanEndBuildTurn() const
{
	return bIsGameStarted_ && !bIsPaused_ && CurrentPhase_ == EGameLoopPhase::Building;
}

bool UGameLoopManager::CanStartCombatEarly() const
{
	if ( !bIsGameStarted_ || bIsPaused_ || CurrentPhase_ != EGameLoopPhase::Building )
	{
		return false;
	}

	return CurrentBuildTurn_ >= 1;
}

bool UGameLoopManager::IsLastWave() const
{
	const int32 total = GetTotalWaves();
	return total > 0 && CurrentWave_ >= total;
}

FText UGameLoopManager::GetEndTurnButtonText() const
{
	if ( CurrentBuildTurn_ >= GetMaxBuildTurns() )
	{
		return FText::FromString( TEXT( "TO BATTLE!" ) );
	}

	return FText::FromString( TEXT( "End Turn" ) );
}

FText UGameLoopManager::GetPhaseDisplayText() const
{
	switch ( CurrentPhase_ )
	{
	case EGameLoopPhase::Startup:
		return FText::FromString( TEXT( "Preparation" ) );
	case EGameLoopPhase::Building:
		return FText::FromString( TEXT( "Building" ) );
	case EGameLoopPhase::Combat:
		return FText::FromString( TEXT( "Combat" ) );
	case EGameLoopPhase::Reward:
		return FText::FromString( TEXT( "Reward" ) );
	case EGameLoopPhase::Victory:
		return FText::FromString( TEXT( "VICTORY!" ) );
	case EGameLoopPhase::Defeat:
		return FText::FromString( TEXT( "DEFEAT" ) );
	case EGameLoopPhase::Paused:
		return FText::FromString( TEXT( "Paused" ) );
	default:
		return FText::FromString( TEXT( "---" ) );
	}
}

FText UGameLoopManager::GetTurnWaveText() const
{
	if ( CurrentPhase_ == EGameLoopPhase::Building )
	{
		return FText::FromString(
		    FString::Printf( TEXT( "Wave %d | Turn %d / %d" ), CurrentWave_, CurrentBuildTurn_, GetMaxBuildTurns() )
		);
	}

	return FText::FromString( FString::Printf( TEXT( "Wave %d / %d" ), CurrentWave_, GetTotalWaves() ) );
}

void UGameLoopManager::SetPhase( EGameLoopPhase newPhase )
{
	if ( newPhase == CurrentPhase_ )
	{
		return;
	}

	EGameLoopPhase oldPhase = CurrentPhase_;
	CurrentPhase_ = newPhase;

	OnPhaseChanged.Broadcast( oldPhase, newPhase );
	BroadcastButtonStates();

	Log( FString::Printf(
	    TEXT( "Phase: %s -> %s" ), *UEnum::GetValueAsString( oldPhase ), *UEnum::GetValueAsString( newPhase )
	) );
}

void UGameLoopManager::EnterStartupPhase()
{
	SetPhase( EGameLoopPhase::Startup );

	GrantStartingResources();

	EnterBuildingPhase();
}

void UGameLoopManager::EnterBuildingPhase()
{
	CurrentBuildTurn_ = 1;
	bPerfectWave_ = true;

	SetPhase( EGameLoopPhase::Building );

	OnBuildTurnChanged.Broadcast( CurrentBuildTurn_, GetMaxBuildTurns() );

	Log( FString::Printf( TEXT( ">>> BUILDING PHASE (Wave %d, Turn 1/%d)" ), CurrentWave_, GetMaxBuildTurns() ) );
}

void UGameLoopManager::EnterCombatPhase()
{
	SetPhase( EGameLoopPhase::Combat );

	bWaveCompleted_ = false;
	LastTimerBroadcast_ = 0.0f;

	const float duration = Config_ ? Config_->CombatDuration : GameLoopDefaults::cDefaultCombatDuration;
	CombatTimeRemaining_ = duration;
	OnCombatTimerUpdated.Broadcast( CombatTimeRemaining_, duration );

	Log( FString::Printf( TEXT( ">>> COMBAT PHASE (Wave %d, Duration: %.1fs)" ), CurrentWave_, CombatTimeRemaining_ ) );

	const float delay = Config_ ? Config_->CombatStartDelay : GameLoopDefaults::cDefaultCombatStartDelay;

	if ( delay > 0.0f )
	{
		if ( UWorld* world = GetWorldSafe() )
		{
			world->GetTimerManager().SetTimer(
			    CombatStartDelayHandle_, this, &UGameLoopManager::StartWave, delay, false
			);
		}
	}
	else
	{
		StartWave();
	}
}

void UGameLoopManager::EnterRewardPhase()
{
	SetPhase( EGameLoopPhase::Reward );

	Log( FString::Printf(
	    TEXT( ">>> REWARD PHASE (Wave %d, Perfect: %s)" ), CurrentWave_, bPerfectWave_ ? TEXT( "YES" ) : TEXT( "NO" )
	) );

	CollectBuildingIncome();

	GrantCombatReward();

	if ( UWorld* world = GetWorldSafe() )
	{
		world->GetTimerManager().SetTimer(
		    BuildingRestoreTimerHandle_, this, &UGameLoopManager::HandleDelayedBuildingRestoration, 1.5f, false
		);
	}

	if ( IsLastWave() )
	{
		EnterVictoryPhase();
		return;
	}

	CurrentWave_++;
	OnWaveChanged.Broadcast( CurrentWave_, GetTotalWaves() );
	EnterBuildingPhase();
}

void UGameLoopManager::EnterVictoryPhase()
{
	SetPhase( EGameLoopPhase::Victory );
	bIsGameStarted_ = false;

	OnGameEnded.Broadcast( true );

	Log( TEXT( "=== VICTORY! ===" ) );

	const FName levelName = FName( TEXT( "Win" ) );

	UWorld* world = GetWorldSafe();

	UGameplayStatics::OpenLevel( world, levelName );
}

void UGameLoopManager::EnterDefeatPhase()
{
	SetPhase( EGameLoopPhase::Defeat );
	bIsGameStarted_ = false;
	UWorld* world = GetWorldSafe();

	if ( world )
	{
		world->GetTimerManager().ClearTimer( CombatStartDelayHandle_ );
	}

	OnGameEnded.Broadcast( false );

	Log( TEXT( "=== DEFEAT ===" ) );

	const FName levelName = FName( TEXT( "Lose" ) );

	UGameplayStatics::OpenLevel( world, levelName );
}

void UGameLoopManager::GrantStartingResources()
{
	if ( !Config_ )
	{
		Log( TEXT( "WARNING: No config for starting resources" ) );
		return;
	}

	if ( !ResourceManager_.IsValid() )
	{
		Log( TEXT( "ERROR: ResourceManager is INVALID!" ) );
		return;
	}

	const FResourceReward& start = Config_->StartingResources;
	UResourceManager* rm = ResourceManager_.Get();

	if ( start.Gold > 0 )
	{
		rm->AddResource( EResourceType::Gold, start.Gold );
	}
	if ( start.Food > 0 )
	{
		rm->AddResource( EResourceType::Food, start.Food );
	}
	if ( start.Population > 0 )
	{
		rm->AddResource( EResourceType::Population, start.Population );
	}

	OnResourcesGranted.Broadcast( start );

	Log( FString::Printf(
	    TEXT( "Starting resources granted: Gold=%d, Food=%d, Pop=%d" ), start.Gold, start.Food, start.Population
	) );
}

void UGameLoopManager::GrantCombatReward()
{
	if ( !Config_ || !ResourceManager_.IsValid() )
	{
		return;
	}

	UResourceManager* rm = ResourceManager_.Get();
	FResourceReward reward = Config_->GetScaledCombatReward( CurrentWave_, bPerfectWave_ );

	if ( reward.Gold > 0 )
	{
		rm->AddResource( EResourceType::Gold, reward.Gold );
	}
	if ( reward.Food > 0 )
	{
		rm->AddResource( EResourceType::Food, reward.Food );
	}
	if ( reward.Population > 0 )
	{
		rm->AddResource( EResourceType::Population, reward.Population );
	}

	OnResourcesGranted.Broadcast( reward );

	Log( FString::Printf(
	    TEXT( "Combat reward: Gold=%d, Food=%d (Perfect: %s)" ), reward.Gold, reward.Food,
	    bPerfectWave_ ? TEXT( "YES" ) : TEXT( "NO" )
	) );
}

void UGameLoopManager::CollectBuildingIncome()
{
	if ( UEconomyComponent* ec = EconomyComponent_.Get() )
	{
		ec->CollectGlobalResources();

		ec->ApplyMaintenanceCosts();

		Log( TEXT( "Collected building income" ) );
	}
}

void UGameLoopManager::StartWave()
{
	if ( AWaveManager* wm = WaveManager_.Get() )
	{
		const int32 waveIndex = CurrentWave_ - 1;
		wm->StartWaveAtIndex( waveIndex );

		Log( FString::Printf( TEXT( "Wave %d started (index %d)" ), CurrentWave_, waveIndex ) );
	}
}

void UGameLoopManager::UpdateCombatTimer( float deltaTime )
{
	if ( CombatTimeRemaining_ <= 0.0f )
	{
		return;
	}

	CombatTimeRemaining_ -= deltaTime;

	if ( CombatTimeRemaining_ <= 0.0f )
	{
		CombatTimeRemaining_ = 0.0f;
		OnCombatTimerUpdated.Broadcast( 0.0f, GetCombatTotalTime() );
		OnCombatTimerExpired();
		return;
	}

	LastTimerBroadcast_ += deltaTime;
	if ( LastTimerBroadcast_ >= GameLoopDefaults::cTimerBroadcastInterval )
	{
		LastTimerBroadcast_ = 0.0f;
		OnCombatTimerUpdated.Broadcast( CombatTimeRemaining_, GetCombatTotalTime() );
	}
}

void UGameLoopManager::OnCombatTimerExpired()
{
	Log( TEXT( "Combat timer expired" ) );

	if ( CurrentPhase_ == EGameLoopPhase::Combat )
	{
		if ( AWaveManager* wM = WaveManager_.Get() )
		{
			wM->CancelCurrentWave();
		}

		EnterRewardPhase();
	}
}

void UGameLoopManager::HandleWaveEnded( int32 waveIndex )
{
	Log( FString::Printf( TEXT( "WaveManager: Wave %d ended" ), waveIndex + 1 ) );

	bWaveCompleted_ = true;
}

void UGameLoopManager::HandleAllWavesCompleted()
{
	Log( TEXT( "WaveManager: All waves completed" ) );
}

void UGameLoopManager::BroadcastButtonStates()
{
	OnButtonStateChanged.Broadcast( FName( "EndTurn" ), CanEndBuildTurn() );
	OnButtonStateChanged.Broadcast( FName( "StartCombat" ), CanStartCombatEarly() );
}

void UGameLoopManager::BindToWaveManager()
{
	if ( bIsBoundToWaveManager_ )
	{
		return;
	}

	if ( AWaveManager* wm = WaveManager_.Get() )
	{
		wm->OnWaveEnded.AddDynamic( this, &UGameLoopManager::HandleWaveEnded );
		wm->OnAllWavesCompleted.AddDynamic( this, &UGameLoopManager::HandleAllWavesCompleted );
		bIsBoundToWaveManager_ = true;

		Log( TEXT( "Bound to WaveManager" ) );
	}
}

void UGameLoopManager::UnbindFromWaveManager()
{
	if ( !bIsBoundToWaveManager_ )
	{
		return;
	}

	if ( AWaveManager* wm = WaveManager_.Get() )
	{
		wm->OnWaveEnded.RemoveDynamic( this, &UGameLoopManager::HandleWaveEnded );
		wm->OnAllWavesCompleted.RemoveDynamic( this, &UGameLoopManager::HandleAllWavesCompleted );
	}

	bIsBoundToWaveManager_ = false;
	Log( TEXT( "Unbound from WaveManager" ) );
}

void UGameLoopManager::Log( const FString& message ) const
{
#if !UE_BUILD_SHIPPING
	UE_LOG( LogGameLoop, Log, TEXT( "[GameLoop] %s" ), *message );
#endif
}

void UGameLoopManager::HandleDelayedBuildingRestoration()
{
	if ( UEconomyComponent* ec = EconomyComponent_.Get() )
	{
		ec->RestoreAllBuildings();
		Log( TEXT( "Delayed building restoration executed." ) );
	}
}
