#pragma once

#include "GameLoopConfig.h"
#include "Tickable.h"

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "GameLoopManager.generated.h"

class AUnitAIManager;
class AWaveManager;
class UCardSubsystem;
class UGameLoopRewardHelper;

namespace GameLoopDefaults
{
	constexpr int32 cDefaultBuildTurns = 2;
	constexpr int32 cDefaultWavesToWin = 10;
	constexpr float cDefaultCombatDuration = 30.0f;
	constexpr float cDefaultCombatStartDelay = 1.5f;

	constexpr float cTimerBroadcastInterval = 0.1f;
} // namespace GameLoopDefaults

UENUM( BlueprintType )
enum class EGameLoopPhase : uint8
{
	None UMETA( DisplayName = "None" ),
	Startup UMETA( DisplayName = "Startup" ),
	Building UMETA( DisplayName = "Building" ),
	Combat UMETA( DisplayName = "Combat" ),
	Reward UMETA( DisplayName = "Reward" ),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnGameLoopPhaseChanged, EGameLoopPhase, OldPhase, EGameLoopPhase, NewPhase
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnGameLoopTurnChanged, int32, CurrentTurn, int32, MaxTurns );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnGameLoopWaveChanged, int32, CurrentWave, int32, TotalWaves );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnGameLoopResourcesGranted, const FResourceReward&, Reward );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnGameLoopCombatTimer, float, TimeRemaining, float, TotalTime );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnGameLoopButtonState, FName, ButtonName, bool, bEnabled );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnLastWaveCompleted, int32, CurrentWave, bool, bPerfectWave );


/**
 * UGameLoopManager
 */
UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UGameLoopManager : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UGameLoopManager();
	virtual ~UGameLoopManager();

	UFUNCTION( BlueprintCallable, Category = "GameLoop|Setup" )
	void InitGameLoop( UGameLoopConfig* inConfig, AUnitAIManager* inUnitAIManager );

	// Game loop control - called by UI or game session controller to start/stop/reset the game. Each validates current
	// phase before executing.
	UFUNCTION( BlueprintCallable, Category = "GameLoop" )
	void StartLoop();

	UFUNCTION( BlueprintCallable, Category = "GameLoop" )
	void StopLoop();

	UFUNCTION( BlueprintCallable, Category = "GameLoop" )
	void Reset();

	// Player actions - called by UI or input handlers. Each validates current phase before executing.
	UFUNCTION( BlueprintCallable, Category = "GameLoop|Actions" )
	void EndBuildTurn();

	UFUNCTION( BlueprintCallable, Category = "GameLoop|Actions" )
	void StartCombatEarly();

	UFUNCTION( BlueprintCallable, Category = "GameLoop|Actions" )
	void ConfirmRewardPhase();

	/// Called by external systems to report significant events that may affect game state.
	UFUNCTION( BlueprintCallable, Category = "GameLoop|Combat" )
	void ReportDamageTaken( float damageAmount );

	UFUNCTION( BlueprintCallable, Category = "GameLoop|Combat" )
	void SetPerfectWave( bool bPerfect )
	{
		bPerfectWave_ = bPerfect;
	}

	UFUNCTION( BlueprintPure, Category = "GameLoop|State" )
	EGameLoopPhase GetCurrentPhase() const
	{
		return CurrentPhase_;
	}

	UFUNCTION( BlueprintPure, Category = "GameLoop|State" )
	int32 GetCurrentWave() const
	{
		return CurrentWave_;
	}

	UFUNCTION( BlueprintPure, Category = "GameLoop|State" )
	int32 GetTotalWaves() const;

	UFUNCTION( BlueprintPure, Category = "GameLoop|State" )
	int32 GetCurrentBuildTurn() const
	{
		return CurrentBuildTurn_;
	}

	UFUNCTION( BlueprintPure, Category = "GameLoop|State" )
	int32 GetMaxBuildTurns() const;

	UFUNCTION( BlueprintPure, Category = "GameLoop|State" )
	int32 GetRemainingBuildTurns() const;

	UFUNCTION( BlueprintPure, Category = "GameLoop|State" )
	float GetCombatTimeRemaining() const
	{
		return CombatTimeRemaining_;
	}

	UFUNCTION( BlueprintPure, Category = "GameLoop|State" )
	float GetCombatTotalTime() const;

	UFUNCTION( BlueprintPure, Category = "GameLoop|Combat" )
	bool IsPerfectWave() const
	{
		return bPerfectWave_;
	}

	UFUNCTION( BlueprintPure, Category = "GameLoop|State" )
	bool IsWaitingForCardSelection() const
	{
		return bWaitingForCardSelection_;
	}

	UFUNCTION( BlueprintPure, Category = "GameLoop|UI" )
	bool CanEndBuildTurn() const;

	UFUNCTION( BlueprintPure, Category = "GameLoop|UI" )
	bool CanStartCombatEarly() const;

	UFUNCTION( BlueprintPure, Category = "GameLoop|UI" )
	bool IsLastWave() const;

	UFUNCTION( BlueprintPure, Category = "GameLoop|UI" )
	FText GetTurnWaveText() const;

	// Delegates
	UPROPERTY( BlueprintAssignable, Category = "GameLoop|Events" )
	FOnGameLoopPhaseChanged OnPhaseChanged;

	UPROPERTY( BlueprintAssignable, Category = "GameLoop|Events" )
	FOnGameLoopTurnChanged OnBuildTurnChanged;

	UPROPERTY( BlueprintAssignable, Category = "GameLoop|Events" )
	FOnGameLoopWaveChanged OnWaveChanged;

	/** Broadcast when resources are granted to player. */
	// UPROPERTY( BlueprintAssignable, Category = "GameLoop|Events" )
	// FOnGameLoopResourcesGranted OnResourcesGranted;

	UPROPERTY( BlueprintAssignable, Category = "GameLoop|Events" )
	FOnGameLoopCombatTimer OnCombatTimerUpdated;

	UPROPERTY( BlueprintAssignable, Category = "GameLoop|Events" )
	FOnGameLoopButtonState OnButtonStateChanged;

	UPROPERTY( BlueprintAssignable, Category = "GameLoop|Events" )
	FOnLastWaveCompleted OnLastWaveCompleted;

	// Tick
	virtual void Tick( float deltaTime ) override;

	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual bool IsTickableInEditor() const override
	{
		return false;
	}
	virtual bool IsTickableWhenPaused() const override
	{
		return false;
	}


protected:
	void SetPhase( EGameLoopPhase newPhase );
	void EnterBuildingPhase();
	void EnterCombatPhase();
	void EnterRewardPhase();

	void StartWave();
	void UpdateCombatTimer( float deltaTime );
	void OnCombatTimerExpired();

	UFUNCTION()
	void HandleWaveEnded( int32 waveIndex );

	UFUNCTION()
	void HandleWaveEndScheduled( float secondsRemaining );

	UFUNCTION()
	void HandleAllWavesCompleted();

	void BroadcastButtonStates();

	void Log( const FString& message ) const;

	void HandleDelayedBuildingRestoration();

	bool IsCardSelectionPending() const;
	void ProceedToNextWave();

private:

	UPROPERTY()
	TObjectPtr<UGameLoopConfig> Config_;

	UPROPERTY()
	TWeakObjectPtr<AWaveManager> WaveManager_;

	UPROPERTY()
	TObjectPtr<UGameLoopRewardHelper> RewardHelper_;

	UPROPERTY()
	TWeakObjectPtr<AUnitAIManager> UnitAIManager_;

	EGameLoopPhase CurrentPhase_ = EGameLoopPhase::None;

	int32 CurrentWave_ = 0;
	int32 CurrentBuildTurn_ = 0;
	float CombatTimeRemaining_ = 0.0f;
	float LastTimerBroadcast_ = 0.0f;

	bool bIsInitialized_ = false;

	bool bPerfectWave_ = true;
	bool bWaitingForCardSelection_ = false;
	bool bIsBoundToWaveManager_ = false;
	FTimerHandle CombatStartDelayHandle_;

	void BindToWaveManager();

	void UnbindFromWaveManager();
};
