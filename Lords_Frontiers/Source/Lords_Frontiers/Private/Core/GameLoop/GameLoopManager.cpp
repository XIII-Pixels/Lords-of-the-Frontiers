#include "Core/GameLoop/GameLoopManager.h"
#include "AI/Path/PathPointsManager.h"

#include "AI/UnitAIManager.h"
#include "Cards/CardPoolConfig.h"
#include "Cards/CardSubsystem.h"
#include "Core/CoreManager.h"
#include "Core/GameLoop/GameLoopRewardHelper.h"
#include "TimerManager.h"
#include "Waves/WaveManager.h"

DEFINE_LOG_CATEGORY_STATIC( LogGameLoop, Log, All );

UGameLoopManager::UGameLoopManager()
{
}

UGameLoopManager::~UGameLoopManager()
{
}

void UGameLoopManager::InitGameLoop( UGameLoopConfig* inConfig, AUnitAIManager* inUnitAIManager )
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

	UnitAIManager_ = inUnitAIManager;

	if ( bIsBoundToWaveManager_ && !WaveManager_.IsValid() )
	{
		bIsBoundToWaveManager_ = false;
	}

	UCoreManager* core = GetGameInstance()->GetSubsystem<UCoreManager>();
	if ( core )
	{
		WaveManager_ = core->GetWaveManager();

		if ( !RewardHelper_ )
		{
			RewardHelper_ = NewObject<UGameLoopRewardHelper>( this );
		}

		RewardHelper_->Initialize( Config_, core->GetResourceManager(), core->GetEconomyComponent() );
	}

	if ( WaveManager_.IsValid() && !bIsBoundToWaveManager_ )
	{
		BindToWaveManager();
	}

	bIsInitialized_ = Config_ != nullptr && WaveManager_.IsValid() && RewardHelper_ != nullptr;

	Log( FString::Printf(
	    TEXT( "Initialized. Config: %s, WaveManager: %s, UnitAIManager: %s" ),
	    Config_ ? TEXT( "OK" ) : TEXT( "MISSING" ), WaveManager_.IsValid() ? TEXT( "OK" ) : TEXT( "MISSING" ),
	    UnitAIManager_.IsValid() ? TEXT( "OK" ) : TEXT( "MISSING" )
	) );
}

void UGameLoopManager::StartLoop()
{
	bWaitingForCardSelection_ = false;

	CurrentPhase_ = EGameLoopPhase::None;
	CurrentWave_ = 1;
	CurrentBuildTurn_ = 0;
	bPerfectWave_ = true;

	Log( TEXT( "=== GAME STARTED ===" ) );
	RewardHelper_->GrantStartingResources();

	OnWaveChanged.Broadcast( CurrentWave_, GetTotalWaves() );
	EnterBuildingPhase();
}

void UGameLoopManager::Reset()
{
	Log( TEXT( "=== GAME RESTARTING ===" ) );

	UnbindFromWaveManager();

	CurrentPhase_ = EGameLoopPhase::None;
	CurrentWave_ = 0;
	CurrentBuildTurn_ = 0;
	CombatTimeRemaining_ = 0.0f;
	bPerfectWave_ = true;
	bWaitingForCardSelection_ = false;

	if ( UWorld* world = GetWorld() )
	{
		world->GetTimerManager().ClearTimer( CombatStartDelayHandle_ );
	}

	if ( WaveManager_.IsValid() )
	{
		BindToWaveManager();
	}
}

void UGameLoopManager::StopLoop()
{
	if ( UWorld* world = GetWorld() )
	{
		world->GetTimerManager().ClearTimer( CombatStartDelayHandle_ );
	}
	CurrentPhase_ = EGameLoopPhase::None;

	Log( TEXT( "Loop stopped" ) );
}

void UGameLoopManager::EndBuildTurn()
{
	if ( !CanEndBuildTurn() )
	{
		Log( TEXT( "WARNING: Cannot end build turn now" ) );
		return;
	}

	if ( CurrentBuildTurn_ >= GetMaxBuildTurns() )
	{
		EnterCombatPhase();
		return;
	}

	RewardHelper_->CollectBuildingIncome();
	RewardHelper_->ApplyMaintenanceCosts();
	CurrentBuildTurn_++;
	OnBuildTurnChanged.Broadcast( CurrentBuildTurn_, GetMaxBuildTurns() );
	BroadcastButtonStates();

	Log( FString::Printf( TEXT( "Build Turn %d / %d" ), CurrentBuildTurn_, GetMaxBuildTurns() ) );
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

void UGameLoopManager::ConfirmRewardPhase()
{
	if ( CurrentPhase_ != EGameLoopPhase::Reward )
	{
		Log( TEXT( "WARNING: ConfirmRewardPhase called outside Reward phase, ignoring" ) );
		return;
	}

	bWaitingForCardSelection_ = false;

	Log( TEXT( "Card selection confirmed, proceeding to next wave" ) );

	ProceedToNextWave();
}

void UGameLoopManager::ProceedToNextWave()
{
	CurrentWave_++;
	OnWaveChanged.Broadcast( CurrentWave_, GetTotalWaves() );
	EnterBuildingPhase();
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
	return CurrentPhase_ == EGameLoopPhase::Building;
}

bool UGameLoopManager::CanStartCombatEarly() const
{
	return CurrentPhase_ == EGameLoopPhase::Building && CurrentBuildTurn_ >= 1;
}

bool UGameLoopManager::IsLastWave() const
{
	const int32 total = GetTotalWaves();
	return total > 0 && CurrentWave_ >= total;
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

void UGameLoopManager::Tick( float deltaTime )
{
	if ( CurrentPhase_ == EGameLoopPhase::Combat )
	{
		UpdateCombatTimer( deltaTime );
	}
}

TStatId UGameLoopManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT( UGameLoopManager, STATGROUP_Tickables );
}

bool UGameLoopManager::IsTickable() const
{
	return bIsInitialized_ && CurrentPhase_ != EGameLoopPhase::None;
}

// protected:

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

void UGameLoopManager::EnterBuildingPhase()
{
	CurrentBuildTurn_ = 1;
	bPerfectWave_ = true;

	SetPhase( EGameLoopPhase::Building );

	OnBuildTurnChanged.Broadcast( CurrentBuildTurn_, GetMaxBuildTurns() );

	Log( FString::Printf( TEXT( ">>> BUILDING PHASE (Wave %d, Turn 1/%d)" ), CurrentWave_, GetMaxBuildTurns() ) );

	RewardHelper_->RecalculateIncome();
}

void UGameLoopManager::EnterCombatPhase()
{
	SetPhase( EGameLoopPhase::Combat );

	LastTimerBroadcast_ = 0.0f;

	const float duration = Config_ ? Config_->CombatDuration : GameLoopDefaults::cDefaultCombatDuration;
	CombatTimeRemaining_ = duration;
	OnCombatTimerUpdated.Broadcast( CombatTimeRemaining_, duration );

	const float delay = Config_ ? Config_->CombatStartDelay : GameLoopDefaults::cDefaultCombatStartDelay;

	if ( delay > 0.0f )
	{
		if ( UWorld* world = GetWorld() )
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

	Log( FString::Printf( TEXT( ">>> COMBAT PHASE (Wave %d, Duration: %.1fs)" ), CurrentWave_, duration ) );
}

void UGameLoopManager::EnterRewardPhase()
{
	SetPhase( EGameLoopPhase::Reward );

	Log( FString::Printf(
	    TEXT( ">>> REWARD PHASE (Wave %d, Perfect: %s)" ), CurrentWave_, bPerfectWave_ ? TEXT( "YES" ) : TEXT( "NO" )
	) );

	RewardHelper_->CollectBuildingIncome();
	RewardHelper_->ApplyMaintenanceCosts();
	RewardHelper_->GrantCombatReward( CurrentWave_, bPerfectWave_ );
	RewardHelper_->RestoreBuildings();

	if ( IsLastWave() )
	{
		OnLastWaveCompleted.Broadcast( CurrentWave_, bPerfectWave_ );
		return;
	}

	if ( IsCardSelectionPending() )
	{
		bWaitingForCardSelection_ = true;
		return;
	}

	ProceedToNextWave();
}

void UGameLoopManager::StartWave()
{
	if ( AWaveManager* wm = WaveManager_.Get() )
	{
		if ( UnitAIManager_.IsValid() )
		{
			UnitAIManager_->OnPreWaveStart();
			UnitAIManager_->PathPointsManager()->ShowAll();
		}

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
	if ( UnitAIManager_.IsValid() )
	{
		UnitAIManager_->PathPointsManager()->Empty();
	}

	Log( FString::Printf( TEXT( "WaveManager: Wave %d ended" ), waveIndex + 1 ) );
}

void UGameLoopManager::HandleWaveEndScheduled( float secondsRemaining )
{
	if ( GetWorld() )
	{
		GetWorld()->GetTimerManager().ClearTimer( CombatStartDelayHandle_ );
	}

	CombatTimeRemaining_ = secondsRemaining;

	OnCombatTimerUpdated.Broadcast( CombatTimeRemaining_, CombatTimeRemaining_ );
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

void UGameLoopManager::Log( const FString& message ) const
{
#if !UE_BUILD_SHIPPING
	UE_LOG( LogGameLoop, Log, TEXT( "[GameLoop] %s" ), *message );
#endif
}

void UGameLoopManager::HandleDelayedBuildingRestoration()
{
	RewardHelper_->RestoreBuildings();
}

bool UGameLoopManager::IsCardSelectionPending() const
{
	UCardSubsystem* cardSub = UCardSubsystem::Get(GetWorld());
	if ( !cardSub )
	{
		return false;
	}

	UCardPoolConfig* poolConfig = cardSub->GetPoolConfig();
	return poolConfig && poolConfig->HasEnoughCards();
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
		wm->OnWaveEndScheduled.AddUniqueDynamic( this, &UGameLoopManager::HandleWaveEndScheduled );
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
		wm->OnWaveEndScheduled.RemoveDynamic( this, &UGameLoopManager::HandleWaveEndScheduled );
	}

	bIsBoundToWaveManager_ = false;
	Log( TEXT( "Unbound from WaveManager" ) );
}
