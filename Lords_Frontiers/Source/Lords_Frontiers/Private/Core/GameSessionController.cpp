// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/GameSessionController.h"

#include "Core/CoreManager.h"
#include "Core/DefaultGameInstance.h"
#include "Core/GameLoop/GameLoopManager.h"
#include "Core/Saving/GameSaveData.h"
#include "Core/Saving/GameSaver.h"
#include "Core/Subsystems/LevelSubsystem/LevelSubsystem.h"
#include "Core/Subsystems/SessionLogger/SessionLoggerSubsystem.h"

#include "Sound/MusicAmbientManager.h"

class UMusicAmbientManager;
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

void UGameSessionController::EndGame( EGameResult result )
{
	bIsGameStarted_ = false;

	if ( UGameInstance* gi = GetGameInstance() )
	{
		if ( UGameSessionController* session = gi->GetSubsystem<UGameSessionController>() )
		{
			session->ResetSpeed();
		}
	}

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
		// No special phase for abandoned games, just reset to main menu or similar.
		break;
	default:
		break;
	}
	if ( GameLoopManager_ )
	{
		GameLoopManager_->StopLoop();
	}
	OnGameEndDelegate.Broadcast( result );
}

void UGameSessionController::EnterVictoryPhase()
{
	PlayWinMusicAndAmbient();
}

void UGameSessionController::EnterDefeatPhase()
{
	PlayLoseMusicAndAmbient();
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

void UGameSessionController::PlayWinMusicAndAmbient() const
{
	if ( const auto* gi = Cast<UDefaultGameInstance>( GetGameInstance() ) )
	{
		if ( auto* musicManager = gi->GetSubsystem<UMusicAmbientManager>() )
		{
			musicManager->StopAllAmbient();
			musicManager->PlayWinGameMusic();
		}
	}
}

void UGameSessionController::PlayLoseMusicAndAmbient() const
{
	if ( const auto* gi = Cast<UDefaultGameInstance>( GetGameInstance() ) )
	{
		if ( auto* musicManager = gi->GetSubsystem<UMusicAmbientManager>() )
		{
			musicManager->StopAllAmbient();
			musicManager->PlayLoseGameMusic();
		}
	}
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
