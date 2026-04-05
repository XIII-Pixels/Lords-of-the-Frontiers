// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/GameSessionController.h"
#include "Core/Subsystems/SessionLogger/SessionLoggerSubsystem.h"
#include "Core/CoreManager.h"
#include "Core/GameLoop/GameLoopManager.h"

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

	switch ( result )
	{
	case EGameResult::Win:
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
		ResetSpeed();
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

void UGameSessionController::ResetSpeed()
{
	SetTimerScale( 1.0f );
	OnSpeedChanged.Broadcast( 1.0f );
}