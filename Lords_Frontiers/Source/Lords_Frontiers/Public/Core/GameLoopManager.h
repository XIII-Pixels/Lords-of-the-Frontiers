#pragma once

#include "AI/Path/PathPointsManager.h"
#include "GameLoopConfig.h"
#include "Tickable.h"

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "GameLoopManager.generated.h"

// Forward declarations
class AWaveManager;
class UResourceManager;
class UEconomyComponent;
class UGameInstance;
class UCardSubsystem;
class UPathPointsManager;

/**
 * Default values used when config is not available.
 * Defined as constexpr for compile-time evaluation and zero runtime cost.
 * Prefix 'c' indicates constant per style guide.
 */
namespace GameLoopDefaults
{
	constexpr int32 cDefaultBuildTurns = 2;
	constexpr int32 cDefaultWavesToWin = 10;
	constexpr float cDefaultCombatDuration = 30.0f;
	constexpr float cDefaultCombatStartDelay = 1.5f;

	/** UI update throttle interval. 100ms is sufficient for timer display. */
	constexpr float cTimerBroadcastInterval = 0.1f;
} // namespace GameLoopDefaults

// ============================================================================
// Phase Enum
// ============================================================================

/**
 * EGameLoopPhase
 *
 * Represents all possible states of the game loop state machine.
 * Transitions are managed by UGameLoopManager.
 *
 * State flow:
 *   None -> Startup -> Building <-> Combat -> Reward -> [CardSelection] -> Building (loop)
 *                                         \-> Victory (if last wave)
 *                         Combat -> Defeat (if base destroyed)
 *                         Any -> Paused -> (previous state)
 */
UENUM( BlueprintType )
enum class EGameLoopPhase : uint8
{
	None UMETA( DisplayName = "None" ),         // Initial state, game not started
	Startup UMETA( DisplayName = "Startup" ),   // Granting starting resources
	Building UMETA( DisplayName = "Building" ), // Player building/upgrading towers
	Combat UMETA( DisplayName = "Combat" ),     // Enemies spawning and attacking
	Reward UMETA( DisplayName = "Reward" ),     // Post-combat, granting rewards + card selection
	Victory UMETA( DisplayName = "Victory" ),   // Player won the game
	Defeat UMETA( DisplayName = "Defeat" ),     // Base destroyed, game over
	Paused UMETA( DisplayName = "Paused" )      // Game paused by player
};

/** Fired when game phase changes. UI should update display accordingly. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnGameLoopPhaseChanged, EGameLoopPhase, OldPhase, EGameLoopPhase, NewPhase
);

/** Fired when build turn changes. UI should update turn counter. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnGameLoopTurnChanged, int32, CurrentTurn, int32, MaxTurns );

/** Fired when wave number changes. UI should update wave counter. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnGameLoopWaveChanged, int32, CurrentWave, int32, TotalWaves );

/** Fired when resources are granted. UI can show reward popup. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnGameLoopResourcesGranted, const FResourceReward&, Reward );

/** Fired periodically during combat. UI should update timer display. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnGameLoopCombatTimer, float, TimeRemaining, float, TotalTime );

/** Fired when game ends. Parameter indicates victory (true) or defeat (false). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnGameLoopEnded, bool, bVictory );

/** Fired when button availability changes. UI should enable/disable buttons. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnGameLoopButtonState, FName, ButtonName, bool, bEnabled );

/**
 * UGameLoopManager
 *
 * Core game loop controller implementing a state machine for tower defense gameplay.
 * Manages phase transitions, timing, rewards, and coordinates with other systems.
 *
 * Architecture:
 * - Owned by UCoreManager (GameInstanceSubsystem)
 * - Implements FTickableGameObject for frame-based timer updates
 * - Uses weak pointers to external systems to avoid preventing GC
 * - Communicates via delegates to maintain loose coupling
 * - Integrates with UCardSubsystem for post-wave card selection
 *
 * Game Loop Cycle:
 *   Startup -> Building (N turns) -> Combat -> Reward -> [Card Selection] -> Building -> ...
 *
 * Dependencies:
 * - UGameLoopConfig: Tunable parameters
 * - AWaveManager: Enemy spawning
 * - UResourceManager: Resource tracking
 * - UEconomyComponent: Building income collection
 * - UCardSubsystem: Post-wave card selection (optional)
 *
 * Thread Safety: Not thread-safe. Must be called from game thread only.
 */
UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UGameLoopManager : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UGameLoopManager();
	virtual ~UGameLoopManager();

	/**
	 * Initializes the game loop manager with required dependencies.
	 * Must be called before StartGame(). Safe to call multiple times.
	 *
	 * @param inConfig - Configuration asset. If null, creates default config.
	 * @param inWaveManager - Wave spawning system. Can be null initially.
	 * @param inResourceManager - Resource tracking. Required for initialization success.
	 * @param inEconomyComponent - Building income. Optional but recommended.
	 */
	UFUNCTION( BlueprintCallable, Category = "GameLoop|Setup" )
	void Initialize(
	    UGameLoopConfig* inConfig, AWaveManager* inWaveManager, UResourceManager* inResourceManager,
	    UEconomyComponent* inEconomyComponent, UPathPointsManager* inPathPointsManager
	);

	/**
	 * Hot-swaps configuration without full reinitialization.
	 * Use when loading different difficulty presets mid-session.
	 * Changes apply to next wave, not current.
	 */
	UFUNCTION( BlueprintCallable, Category = "GameLoop|Setup" )
	void SetConfig( UGameLoopConfig* newConfig );

	/**
	 * Updates system references without resetting game state.
	 * Called by CoreManager when world actors are recreated (level transitions).
	 */
	UFUNCTION( BlueprintCallable, Category = "GameLoop|Setup" )
	void UpdateDependencies(
	    AWaveManager* inWaveManager, UResourceManager* inResourceManager, UEconomyComponent* inEconomyComponent
	);

	/**
	 * Releases all resources and unbinds from external systems.
	 * Called automatically by destructor, but can be called manually
	 * for controlled shutdown order.
	 */
	UFUNCTION( BlueprintCallable, Category = "GameLoop|Setup" )
	void Cleanup();

	/** Returns true if Initialize() completed successfully. */
	UFUNCTION( BlueprintPure, Category = "GameLoop|Setup" )
	bool IsInitialized() const
	{
		return bIsInitialized_;
	}

	/** Called every frame. Updates combat timer when in Combat phase. */
	virtual void Tick( float deltaTime ) override;

	/** Returns true if ticking should occur. False when paused or not playing. */
	virtual bool IsTickable() const override;

	/** Editor ticking disabled - no preview needed. */
	virtual bool IsTickableInEditor() const override
	{
		return false;
	}

	/** Respects global pause state. */
	virtual bool IsTickableWhenPaused() const override
	{
		return false;
	}

	/** Returns stat ID for Unreal Insights profiling. */
	virtual TStatId GetStatId() const override;

	/** Returns world for tick registration. Required by tickable interface. */
	virtual UWorld* GetTickableGameObjectWorld() const override;

	/**
	 * Starts a new game session.
	 * Grants starting resources and enters Building phase.
	 * Requires: IsInitialized() == true
	 */
	UFUNCTION( BlueprintCallable, Category = "GameLoop" )
	void StartGame();

	/**
	 * Resets all state and starts fresh.
	 * Use for "Play Again" functionality.
	 */
	UFUNCTION( BlueprintCallable, Category = "GameLoop" )
	void RestartGame();

	/**
	 * Pauses the game loop. Timer stops, no phase transitions.
	 * Stores current phase to restore on resume.
	 */
	UFUNCTION( BlueprintCallable, Category = "GameLoop" )
	void PauseGame();

	/**
	 * Resumes from pause. Restores previous phase.
	 */
	UFUNCTION( BlueprintCallable, Category = "GameLoop" )
	void ResumeGame();

	/**
	 * Player ends current build turn.
	 * If all turns used, transitions to Combat phase.
	 * Otherwise, collects income and advances turn counter.
	 */
	UFUNCTION( BlueprintCallable, Category = "GameLoop|Actions" )
	void EndBuildTurn();

	/**
	 * Player skips remaining build turns and starts combat immediately.
	 * Requires at least 1 build turn completed (prevents instant-start exploits).
	 */
	UFUNCTION( BlueprintCallable, Category = "GameLoop|Actions" )
	void StartCombatEarly();

	/**
	 * Called by MainBase when it takes damage from enemies.
	 * Any damage > 0 disqualifies the wave from "perfect" bonus.
	 */
	UFUNCTION( BlueprintCallable, Category = "GameLoop|Combat" )
	void ReportDamageTaken( float damageAmount );

	/** Directly sets perfect wave flag. For testing/cheats. */
	UFUNCTION( BlueprintCallable, Category = "GameLoop|Combat" )
	void SetPerfectWave( bool bPerfect )
	{
		bPerfectWave_ = bPerfect;
	}

	/** Returns true if base has taken no damage this wave. */
	UFUNCTION( BlueprintPure, Category = "GameLoop|Combat" )
	bool IsPerfectWave() const
	{
		return bPerfectWave_;
	}

	/** Current phase of the game loop state machine. */
	UFUNCTION( BlueprintPure, Category = "GameLoop|State" )
	EGameLoopPhase GetCurrentPhase() const
	{
		return CurrentPhase_;
	}

	/** Current wave number (1-indexed). */
	UFUNCTION( BlueprintPure, Category = "GameLoop|State" )
	int32 GetCurrentWave() const
	{
		return CurrentWave_;
	}

	/** Total waves to complete for victory. */
	UFUNCTION( BlueprintPure, Category = "GameLoop|State" )
	int32 GetTotalWaves() const;

	/** Current build turn within the building phase (1-indexed). */
	UFUNCTION( BlueprintPure, Category = "GameLoop|State" )
	int32 GetCurrentBuildTurn() const
	{
		return CurrentBuildTurn_;
	}

	/** Maximum build turns allowed per wave. */
	UFUNCTION( BlueprintPure, Category = "GameLoop|State" )
	int32 GetMaxBuildTurns() const;

	/** Build turns remaining before forced combat. */
	UFUNCTION( BlueprintPure, Category = "GameLoop|State" )
	int32 GetRemainingBuildTurns() const;

	/** Seconds remaining in combat phase. */
	UFUNCTION( BlueprintPure, Category = "GameLoop|State" )
	float GetCombatTimeRemaining() const
	{
		return CombatTimeRemaining_;
	}

	/** Total combat duration for this wave. */
	UFUNCTION( BlueprintPure, Category = "GameLoop|State" )
	float GetCombatTotalTime() const;

	/** True if game is currently in progress. */
	UFUNCTION( BlueprintPure, Category = "GameLoop|State" )
	bool IsGameStarted() const
	{
		return bIsGameStarted_;
	}

	/** True if game is paused. */
	UFUNCTION( BlueprintPure, Category = "GameLoop|State" )
	bool IsGamePaused() const
	{
		return bIsPaused_;
	}

	/** True if waiting for player to select cards in Reward phase. */
	UFUNCTION( BlueprintPure, Category = "GameLoop|State" )
	bool IsWaitingForCardSelection() const
	{
		return bWaitingForCardSelection_;
	}

	/** Returns true if EndBuildTurn() can be called. For button enable state. */
	UFUNCTION( BlueprintPure, Category = "GameLoop|UI" )
	bool CanEndBuildTurn() const;

	/** Returns true if StartCombatEarly() can be called. For button enable state. */
	UFUNCTION( BlueprintPure, Category = "GameLoop|UI" )
	bool CanStartCombatEarly() const;

	/** Returns true if current wave is the final wave. */
	UFUNCTION( BlueprintPure, Category = "GameLoop|UI" )
	bool IsLastWave() const;

	/** Returns localized button text ("End Turn" or "To Battle!"). */
	UFUNCTION( BlueprintPure, Category = "GameLoop|UI" )
	FText GetEndTurnButtonText() const;

	/** Returns localized phase name for display. */
	UFUNCTION( BlueprintPure, Category = "GameLoop|UI" )
	FText GetPhaseDisplayText() const;

	/** Returns formatted string like "Wave 3 | Turn 2/3". */
	UFUNCTION( BlueprintPure, Category = "GameLoop|UI" )
	FText GetTurnWaveText() const;

	/**
	 * Called by MainBase when health reaches zero.
	 * Transitions to Defeat phase and ends game.
	 */
	UFUNCTION( BlueprintCallable, Category = "GameLoop" )
	void EnterDefeatPhase();

	/**
	 * Called after card selection is complete (by CardSubsystem or UI).
	 * Proceeds to next wave and enters Building phase.
	 * Also serves as a fallback if no card system is active.
	 */
	UFUNCTION( BlueprintCallable, Category = "GameLoop|Actions" )
	void ConfirmRewardPhase();

	/** Broadcast when phase changes. Subscribe for UI updates. */
	UPROPERTY( BlueprintAssignable, Category = "GameLoop|Events" )
	FOnGameLoopPhaseChanged OnPhaseChanged;

	/** Broadcast when build turn changes. */
	UPROPERTY( BlueprintAssignable, Category = "GameLoop|Events" )
	FOnGameLoopTurnChanged OnBuildTurnChanged;

	/** Broadcast when wave number changes. */
	UPROPERTY( BlueprintAssignable, Category = "GameLoop|Events" )
	FOnGameLoopWaveChanged OnWaveChanged;

	/** Broadcast when resources are granted to player. */
	UPROPERTY( BlueprintAssignable, Category = "GameLoop|Events" )
	FOnGameLoopResourcesGranted OnResourcesGranted;

	/** Broadcast periodically during combat with remaining time. */
	UPROPERTY( BlueprintAssignable, Category = "GameLoop|Events" )
	FOnGameLoopCombatTimer OnCombatTimerUpdated;

	/** Broadcast when game ends (victory or defeat). */
	UPROPERTY( BlueprintAssignable, Category = "GameLoop|Events" )
	FOnGameLoopEnded OnGameEnded;

	/** Broadcast when button availability changes. */
	UPROPERTY( BlueprintAssignable, Category = "GameLoop|Events" )
	FOnGameLoopButtonState OnButtonStateChanged;

protected:

	/**
	 * Central phase transition method. All phase changes go through here.
	 * Broadcasts OnPhaseChanged and updates button states.
	 */
	void SetPhase( EGameLoopPhase newPhase );

	/** Grants starting resources and immediately transitions to Building. */
	void EnterStartupPhase();

	/** Resets turn counter and prepares for player building. */
	void EnterBuildingPhase();

	/** Starts combat timer and schedules enemy spawning. */
	void EnterCombatPhase();

	void UpdatePostProcessVolume( EGameLoopPhase phase );
	/**
	 * Grants combat rewards, starts healing, and requests card selection.
	 * Does NOT auto-transition to Building. Waits for ConfirmRewardPhase().
	 */
	void EnterRewardPhase();

	/** Sets game as won and broadcasts OnGameEnded(true). */
	void EnterVictoryPhase();

	/** Grants initial resources from config. Called once at game start. */
	void GrantStartingResources();

	/** Grants scaled combat reward based on wave and perfect status. */
	void GrantCombatReward();

	/** Collects passive income from all economy buildings. */
	void CollectBuildingIncome();

	/** Tells WaveManager to start spawning enemies. */
	void StartWave();

	/**
	 * Updates combat timer each frame. Handles:
	 * - Decrementing remaining time
	 * - Throttled UI broadcasts (every 100ms)
	 * - Timer expiration callback
	 */
	void UpdateCombatTimer( float deltaTime );

	/** Called when combat timer reaches zero. Transitions to Reward phase. */
	void OnCombatTimerExpired();

	/** Called when WaveManager completes a wave. */
	UFUNCTION()
	void HandleWaveEnded( int32 waveIndex );

	/** Called when WaveManager has no more waves. */
	UFUNCTION()
	void HandleAllWavesCompleted();

	/** Broadcasts current state of all action buttons. */
	void BroadcastButtonStates();

	/** Logs message to LogGameLoop category. Stripped in shipping builds. */
	void Log( const FString& message ) const;

	/** Safely retrieves World pointer through outer chain. Returns null if unavailable. */
	UWorld* GetWorldSafe() const;

	void HandleDelayedBuildingRestoration();

	void ExecuteHealingPulse();

	/**
	 * Checks if CardSubsystem has a valid pool and can offer cards.
	 * Used to decide whether to wait for card selection in Reward phase.
	 */
	bool IsCardSelectionPending() const;

	/**
	 * Increments wave counter and transitions to Building phase.
	 * Extracted from EnterRewardPhase to support async card selection.
	 */
	void ProceedToNextWave();

private:

	/** Configuration data asset. Owned by this manager if created internally. */
	UPROPERTY()
	TObjectPtr<UGameLoopConfig> Config_;

	/** Weak ref to wave spawning system. May be null between level transitions. */
	UPROPERTY()
	TWeakObjectPtr<AWaveManager> WaveManager_;

	/** Weak ref to resource tracking system. Critical dependency. */
	UPROPERTY()
	TWeakObjectPtr<UResourceManager> ResourceManager_;

	/** Weak ref to economy system for building income. Optional. */
	UPROPERTY()
	TWeakObjectPtr<UEconomyComponent> EconomyComponent_;

	/** Weak ref to path points manager. */
	UPROPERTY()
	TWeakObjectPtr<UPathPointsManager> PathPointsManager_;

	/** Current state machine phase. */
	EGameLoopPhase CurrentPhase_ = EGameLoopPhase::None;

	/** Stored phase before pause, for restoration on resume. */
	EGameLoopPhase PhaseBeforePause_ = EGameLoopPhase::None;

	/** Current wave number (1-indexed). */
	int32 CurrentWave_ = 0;

	/** Current build turn (1-indexed). */
	int32 CurrentBuildTurn_ = 0;

	/** Remaining seconds in combat phase. */
	float CombatTimeRemaining_ = 0.0f;

	/** Accumulator for throttling timer broadcasts. */
	float LastTimerBroadcast_ = 0.0f;

	/** True after successful Initialize(). */
	bool bIsInitialized_ = false;

	/** True while game is in progress. */
	bool bIsGameStarted_ = false;

	/** True while game is paused. */
	bool bIsPaused_ = false;

	/** True if we've subscribed to WaveManager events. */
	bool bIsBoundToWaveManager_ = false;

	/** Set true when WaveManager reports wave completed. */
	bool bWaveCompleted_ = false;

	/** True if base has taken no damage this wave. */
	bool bPerfectWave_ = true;

	/** True while waiting for player to select cards in Reward phase. */
	bool bWaitingForCardSelection_ = false;

	/** Timer handle for delayed wave start after combat begins. */
	FTimerHandle CombatStartDelayHandle_;

	/** Subscribes to WaveManager events. Idempotent. */
	void BindToWaveManager();

	/** Unsubscribes from WaveManager events. Safe to call if not bound. */
	void UnbindFromWaveManager();

	FTimerHandle BuildingRestoreTimerHandle_;

	int32 RemainingHealingPulses_ = 0;
};
