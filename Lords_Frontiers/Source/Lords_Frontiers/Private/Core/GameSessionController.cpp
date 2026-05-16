// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/GameSessionController.h"

#include "Core/CoreManager.h"
#include "Core/GameLoop/GameLoopManager.h"
#include "Core/Saving/GameSaveData.h"
#include "Core/Saving/GameSaver.h"
#include "Core/Subsystems/LevelSubsystem/LevelSubsystem.h"
#include "Core/Subsystems/SessionLogger/SessionLoggerSubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "EntityStats.h"
#include "Lords_Frontiers/Public/Match/MatchScoringConfig.h"
#include "Lords_Frontiers/Public/Match/MatchStatsTracker.h"
#include "Lords_Frontiers/Public/Units/Unit.h"
#include "Lords_Frontiers/Public/Waves/WaveManager.h"

void UGameSessionController::Initialize( FSubsystemCollectionBase& Collection )
{
	Super::Initialize( Collection );
	GameLoopManager_ = GetGameInstance()->GetSubsystem<UGameLoopManager>();

	if ( GameLoopManager_ )
	{
		GameLoopManager_->OnPhaseChanged.AddDynamic( this, &UGameSessionController::HandlePhaseChanged );
		GameLoopManager_->OnLastWaveCompleted.AddDynamic( this, &UGameSessionController::HandleLastWaveCompleted );
	}
}

void UGameSessionController::StartGame()
{
	ResetState();
	bIsGameStarted_ = true;
	if ( GameLoopManager_ )
	{
		GameLoopManager_->StartLoop();
	}
}

void UGameSessionController::ResetState()
{
	bIsGameStarted_ = false;
	bIsGamePaused_ = false;
	LastCombatSpeed_ = 1.0f;
	SetTimerScale( 1.0f );

	if ( USessionLoggerSubsystem* logger = GetWorld()->GetSubsystem<USessionLoggerSubsystem>() )
	{
		logger->FinalizeSessionOnRestart();
	}

	UCoreManager* core = GetGameInstance()->GetSubsystem<UCoreManager>();
	core->ResetGameState();
}

void UGameSessionController::RestartGame()
{
	EndGame( EGameResult::Abandoned );
	if ( GameLoopManager_ )
	{
		GameLoopManager_->Reset();
	}
	// For now, just call StartGame. In the future, we may want to add additional logic here.
	StartGame();
}

bool UGameSessionController::IsInsideEndlessMode() const
{
	UWorld* world = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if ( !world )
	{
		world = GetWorld();
	}
	if ( !world )
	{
		UE_LOG( LogTemp, Warning, TEXT( "IsInsideEndlessMode: world is null." ) );
		return false;
	}

	TArray<AActor*> actors;
	UGameplayStatics::GetAllActorsOfClass( world, AWaveManager::StaticClass(), actors );
	if ( actors.Num() == 0 )
	{
		UE_LOG( LogTemp, Warning, TEXT( "IsInsideEndlessMode: no AWaveManager found in world." ) );
		return false;
	}

	for ( AActor* a : actors )
	{
		const AWaveManager* wm = Cast<AWaveManager>( a );
		if ( !wm )
		{
			continue;
		}
		const bool bByLatch = wm->IsEndlessRunActive();
		const bool bByIndex = wm->HasInfiniteMode() && wm->IsInfiniteWaveIndex( wm->CurrentWaveIndex );
		UE_LOG(
			LogTemp, Log,
			TEXT( "IsInsideEndlessMode: WaveManager '%s' hasInfinite=%d currentWave=%d latch=%d byIndex=%d" ),
			*wm->GetName(), wm->HasInfiniteMode() ? 1 : 0, wm->CurrentWaveIndex,
			bByLatch ? 1 : 0, bByIndex ? 1 : 0
		);
		if ( bByLatch || bByIndex )
		{
			return true;
		}
	}
	return false;
}

void UGameSessionController::EndGame( EGameResult result )
{
	bIsGameStarted_ = false;

	const EGameResult inputResult = result;
	if ( ( result == EGameResult::Win || result == EGameResult::Lose ) && IsInsideEndlessMode() )
	{
		result = EGameResult::EndlessRun;
	}
	UE_LOG(
		LogTemp, Log, TEXT( "GameSessionController::EndGame input=%d final=%d" ),
		static_cast<int32>( inputResult ), static_cast<int32>( result )
	);

	switch ( result )
	{
	case EGameResult::Win:
		if ( const auto* gameInstance = GetGameInstance() )
		{
			if ( const auto* gameSaver = gameInstance->GetSubsystem<UGameSaver>() )
			{
				gameSaver->UpdateCurrentLevelStatus( ELevelStatus::Completed );
			}
			if ( const auto* levelSubsystem = gameInstance->GetSubsystem<ULevelSubsystem>() )
			{
				levelSubsystem->UnlockNextLevel();
			}
		}

		EnterVictoryPhase();
		break;
	case EGameResult::Lose:
		EnterDefeatPhase();
		break;
	case EGameResult::Abandoned:
		break;
	case EGameResult::EndlessRun:
		if ( UGameInstance* gi = GetGameInstance() )
		{
			if ( UMatchStatsTracker* tracker = gi->GetSubsystem<UMatchStatsTracker>() )
			{
				const FString playerName = ( tracker->GetConfig() && !tracker->GetConfig()->PlayerEntryName.IsEmpty() )
												? tracker->GetConfig()->PlayerEntryName
												: TEXT( "Ты" );
				tracker->FinalizeAndPush( playerName );
			}
		}
		break;
	default:
		break;
	}
	if ( GameLoopManager_ )
	{
		GameLoopManager_->StopLoop();
	}

	CleanupBattlefield();

	OnGameEndDelegate.Broadcast( result );
}

void UGameSessionController::CleanupBattlefield()
{
	UWorld* world = GetGameInstance() ? GetGameInstance()->GetWorld() : GetWorld();
	if ( !world )
	{
		return;
	}

	TArray<AActor*> waveManagers;
	UGameplayStatics::GetAllActorsOfClass( world, AWaveManager::StaticClass(), waveManagers );
	for ( AActor* a : waveManagers )
	{
		if ( AWaveManager* wm = Cast<AWaveManager>( a ) )
		{
			wm->CancelCurrentWave();
		}
	}

	TArray<AActor*> units;
	UGameplayStatics::GetAllActorsOfClass( world, AUnit::StaticClass(), units );
	int32 destroyed = 0;
	for ( AActor* a : units )
	{
		AUnit* unit = Cast<AUnit>( a );
		if ( !unit || unit->Team() != ETeam::Dog )
		{
			continue;
		}
		unit->Destroy();
		++destroyed;
	}
	UE_LOG( LogTemp, Log, TEXT( "GameSessionController: cleanup destroyed %d enemy units." ), destroyed );
}

void UGameSessionController::EnterVictoryPhase()
{
}

void UGameSessionController::EnterDefeatPhase()
{
}

void UGameSessionController::PauseGame()
{
	if ( !bIsGameStarted_ || bIsGamePaused_ )
	{
		return;
	}

	bIsGamePaused_ = true;
	OnPauseChanged.Broadcast( bIsGamePaused_ );
}

void UGameSessionController::ResumeGame()
{
	if ( !bIsGamePaused_ )
	{
		return;
	}

	bIsGamePaused_ = false;
	OnPauseChanged.Broadcast( bIsGamePaused_ );
}

void UGameSessionController::SetTimerScale( float newTimeScale )
{
	if ( UWorld* world = GetWorld() )
	{
		world->GetWorldSettings()->SetTimeDilation( newTimeScale );
	}
}

float UGameSessionController::GetTimerScale() const
{
	if ( UWorld* world = GetWorld() )
	{
		return world->GetWorldSettings()->GetEffectiveTimeDilation();
	}
	return 1.0f;
}

void UGameSessionController::HandleLastWaveCompleted( int32 currentWave, bool bPerfectWave )
{
	EndGame( EGameResult::Win );
}

void UGameSessionController::HandlePhaseChanged( EGameLoopPhase oldPhase, EGameLoopPhase newPhase )
{
	if ( oldPhase == EGameLoopPhase::Combat && newPhase != EGameLoopPhase::Combat )
	{
		SetTimerScale( 1.0f );
		OnSpeedChanged.Broadcast( 1.0f );
	}
	else if ( newPhase == EGameLoopPhase::Combat && oldPhase != EGameLoopPhase::Combat )
	{
		SetGameSpeed( LastCombatSpeed_ );
	}
}

void UGameSessionController::CycleSpeed()
{
	if ( !GameLoopManager_ || GameLoopManager_->GetCurrentPhase() != EGameLoopPhase::Combat )
	{
		return;
	}

	const float current = GetTimerScale();
	float next = 1.0f;

	if ( current < 1.5f )
	{
		next = 2.0f;
	}
	else if ( current < 7.0f )
	{
		next = 8.0f;
	}

	SetTimerScale( next );
	OnSpeedChanged.Broadcast( next );
}

void UGameSessionController::SetGameSpeed( float newSpeed )
{
	if ( !GameLoopManager_ || GameLoopManager_->GetCurrentPhase() != EGameLoopPhase::Combat )
	{
		return;
	}

	const float clamped = FMath::Max( newSpeed, 0.0f );
	SetTimerScale( clamped );
	OnSpeedChanged.Broadcast( clamped );

	if ( clamped > 0.0f )
	{
		LastCombatSpeed_ = clamped;
	}
}

void UGameSessionController::ResetSpeed()
{
	SetTimerScale( 1.0f );
	OnSpeedChanged.Broadcast( 1.0f );
}
