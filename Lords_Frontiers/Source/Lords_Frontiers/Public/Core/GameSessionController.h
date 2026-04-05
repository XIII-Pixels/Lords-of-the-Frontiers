// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Core/GameLoop/GameLoopManager.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "CoreMinimal.h"

#include "GameSessionController.generated.h"

UENUM( BlueprintType )
enum class EGameResult : uint8
{
	Win,
	Lose,
	Abandoned
};

class UGameLoopManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnGameEnd, EGameResult, Result );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnGameStart, EGameResult, Result );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnPauseChanged, bool, bIsPaused );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnSpeedChanged, float, NewSpeed );

/**
 *Maxim
 */
UCLASS()
class LORDS_FRONTIERS_API UGameSessionController : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize( FSubsystemCollectionBase& Collection ) override;

	UFUNCTION( BlueprintCallable, Category = "GameSession" )
	void StartGame();

	UFUNCTION( BlueprintCallable, Category = "GameSession" )
	void ResetState();

	UFUNCTION( BlueprintCallable, Category = "GameSession" )
	void RestartGame();

	UFUNCTION( BlueprintCallable, Category = "GameSession" )
	void PauseGame();

	UFUNCTION( BlueprintCallable, Category = "GameSession" )
	void ResumeGame();

	UFUNCTION( BlueprintCallable, Category = "GameSession" )
	void EndGame( EGameResult result );

	UFUNCTION( BlueprintCallable, Category = "GameSession" )
	void SetTimerScale( float newTimeScale );

	UFUNCTION( BlueprintCallable, Category = "GameSession" )
	float GetTimerScale() const;

	UFUNCTION( BlueprintCallable, Category = "GameSession|Speed" )
	void CycleSpeed();

	UFUNCTION( BlueprintCallable, Category = "GameSession|Speed" )
	void ResetSpeed();

	UFUNCTION( BlueprintPure, Category = "Session" )
	bool IsGameStarted() const
	{
		return bIsGameStarted_;
	}

	UFUNCTION( BlueprintPure, Category = "Session" )
	bool IsGamePaused() const
	{
		return bIsGamePaused_;
	}

	UPROPERTY( BlueprintAssignable, Category = "GameSession|Events" )
	FOnGameEnd OnGameEndDelegate;

	UPROPERTY( BlueprintAssignable, Category = "GameSession|Events" )
	FOnPauseChanged OnPauseChanged;

	UPROPERTY( BlueprintAssignable, Category = "GameSession|Events" )
	FOnSpeedChanged OnSpeedChanged;

private:
	void EnterVictoryPhase();
	void EnterDefeatPhase();

	UFUNCTION()
	void HandlePhaseChanged( EGameLoopPhase OldPhase, EGameLoopPhase NewPhase );

	UPROPERTY()
	TObjectPtr<UGameLoopManager> GameLoopManager_;

	bool bIsGameStarted_ = false;
	bool bIsGamePaused_ = false;
};
