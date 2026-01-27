#pragma once

#include "Subsystems/GameInstanceSubsystem.h"

#include "CoreMinimal.h"

#include "CoreManager.generated.h"

// Forward declarations - minimizes header dependencies and compile times
class AWaveManager;
class ABuildManager;
class AGridVisualizer;
class UResourceManager;
class UEconomyComponent;
class USelectionManagerComponent;
class UGameLoopManager;
class UGameLoopConfig;
class UWorld;

/** Broadcast when all critical systems are initialized and ready. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnCoreSystemsReady );

/**
 * UCoreManager
 *
 * Central service locator and lifecycle manager for all game systems.
 * Implemented as a GameInstanceSubsystem for automatic lifecycle management.
 *
 * Architecture:
 * - Singleton-like access via Get() with world context
 * - Owns UGameLoopManager (created internally)
 * - Holds weak references to world actors (WaveManager, BuildManager, etc.)
 * - Holds weak references to PlayerController components (ResourceManager, etc.)
 *
 * Lifecycle:
 * 1. Engine creates UCoreManager when GameInstance initializes
 * 2. GameMode calls InitializeSystems() at game start
 * 3. UCoreManager finds/creates all required systems
 * 4. OnSystemsReady broadcasts when critical systems are available
 * 5. Systems remain available until level unload or game exit
 *
 * Design Rationale:
 * - Subsystem approach: Automatic creation, no manual spawning needed
 * - Weak pointers: Don't prevent GC of actors during level transitions
 * - Service locator pattern: Decouples systems from each other
 * - Template helpers: Reduce registration boilerplate, ensure consistent behavior
 *
 * Thread Safety: Not thread-safe. Access from game thread only.
 */
UCLASS()
class LORDS_FRONTIERS_API UCoreManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	/**
	 * Retrieves the CoreManager instance from any world context object.
	 *
	 * Usage:
	 *   UCoreManager* Core = UCoreManager::Get(this);
	 *   if (Core) { UResourceManager* RM = Core->GetResourceManager(); }
	 *
	 * @param worldContextObject - Any UObject with a valid world (Actor, Component, etc.)
	 * @return CoreManager instance, or nullptr if not available
	 */
	UFUNCTION( BlueprintPure, Category = "Core", meta = ( WorldContext = "WorldContextObject" ) )
	static UCoreManager* Get( const UObject* worldContextObject );

	/**
	 * Called by engine when GameInstance is created.
	 * Performs minimal setup - heavy initialization deferred to InitializeSystems().
	 */
	virtual void Initialize( FSubsystemCollectionBase& collection ) override;

	/**
	 * Called by engine on shutdown.
	 * Cleans up GameLoopManager and clears all references.
	 */
	virtual void Deinitialize() override;

	/**
	 * Determines if this subsystem should be created.
	 * Always returns true - CoreManager is always needed.
	 */
	virtual bool ShouldCreateSubsystem( UObject* outer ) const override
	{
		return true;
	}

	/**
	 * Main initialization entry point. Call from GameMode::BeginPlay().
	 *
	 * Performs:
	 * 1. FindWorldActors() - Locates manager actors in world
	 * 2. FindPlayerControllerComponents() - Finds/creates PC components
	 * 3. CreateInternalManagers() - Creates owned managers (GameLoopManager)
	 * 4. SetupManagerConnections() - Wires dependencies between managers
	 *
	 * Broadcasts OnSystemsReady if critical systems available.
	 */
	UFUNCTION( BlueprintCallable, Category = "Core|Setup" )
	void InitializeSystems();

	/**
	 * Refreshes system references without full reset.
	 *
	 * Use cases:
	 * - After seamless travel
	 * - When actors are respawned
	 * - When player controller changes
	 *
	 * Does NOT reset GameLoopManager state or config.
	 */
	UFUNCTION( BlueprintCallable, Category = "Core|Setup" )
	void RefreshSystemReferences();

	/**
	 * Full system reset. Cleans up everything.
	 * Use when completely restarting the game session.
	 */
	UFUNCTION( BlueprintCallable, Category = "Core|Setup" )
	void ResetSystems();

	/**
	 * Returns true if ALL systems are initialized and valid.
	 * Use for full feature availability checks.
	 */
	UFUNCTION( BlueprintPure, Category = "Core|Setup" )
	bool AreAllSystemsReady() const;

	/**
	 * Returns true if critical systems are ready.
	 * Critical = ResourceManager + GameLoopManager
	 * Game can start with only critical systems.
	 */
	UFUNCTION( BlueprintPure, Category = "Core|Setup" )
	bool AreCriticalSystemsReady() const;

	/** Returns true after InitializeSystems() has completed. */
	UFUNCTION( BlueprintPure, Category = "Core|Setup" )
	bool IsInitialized() const
	{
		return bIsInitialized_;
	}

	/** Returns WaveManager actor. May be null if not placed in level. */
	UFUNCTION( BlueprintPure, Category = "Core|Managers" )
	AWaveManager* GetWaveManager() const;

	/** Returns BuildManager actor. May be null if not placed in level. */
	UFUNCTION( BlueprintPure, Category = "Core|Managers" )
	ABuildManager* GetBuildManager() const;

	/** Returns GridVisualizer actor. May be null if not placed in level. */
	UFUNCTION( BlueprintPure, Category = "Core|Managers" )
	AGridVisualizer* GetGridVisualizer() const;

	/** Returns ResourceManager component. Created if not found on PlayerController. */
	UFUNCTION( BlueprintPure, Category = "Core|Managers" )
	UResourceManager* GetResourceManager() const;

	/** Returns EconomyComponent. Created if not found on PlayerController. */
	UFUNCTION( BlueprintPure, Category = "Core|Managers" )
	UEconomyComponent* GetEconomyComponent() const;

	/** Returns SelectionManager component. Optional - may be null. */
	UFUNCTION( BlueprintPure, Category = "Core|Managers" )
	USelectionManagerComponent* GetSelectionManager() const;

	/** Returns GameLoopManager. Always created internally. */
	UFUNCTION( BlueprintPure, Category = "Core|Managers" )
	UGameLoopManager* GetGameLoop() const;

	/**
	 * Registers a WaveManager with CoreManager.
	 * Called by WaveManager::BeginPlay().
	 *
	 * @param waveManager - The WaveManager to register
	 */
	UFUNCTION( BlueprintCallable, Category = "Core|Registration" )
	void RegisterWaveManager( AWaveManager* waveManager );

	/**
	 * Unregisters a WaveManager.
	 * Called by WaveManager::EndPlay().
	 */
	UFUNCTION( BlueprintCallable, Category = "Core|Registration" )
	void UnregisterWaveManager( AWaveManager* waveManager );

	/** Registers BuildManager. Called from BuildManager::BeginPlay(). */
	UFUNCTION( BlueprintCallable, Category = "Core|Registration" )
	void RegisterBuildManager( ABuildManager* buildManager );

	/** Unregisters BuildManager. Called from BuildManager::EndPlay(). */
	UFUNCTION( BlueprintCallable, Category = "Core|Registration" )
	void UnregisterBuildManager( ABuildManager* buildManager );

	/** Registers GridVisualizer. Called from GridVisualizer::BeginPlay(). */
	UFUNCTION( BlueprintCallable, Category = "Core|Registration" )
	void RegisterGridVisualizer( AGridVisualizer* gridVisualizer );

	/** Unregisters GridVisualizer. Called from GridVisualizer::EndPlay(). */
	UFUNCTION( BlueprintCallable, Category = "Core|Registration" )
	void UnregisterGridVisualizer( AGridVisualizer* gridVisualizer );

	/**
	 * Broadcast when critical systems become ready.
	 * Subscribe to this to safely access managers after initialization.
	 */
	UPROPERTY( BlueprintAssignable, Category = "Core|Events" )
	FOnCoreSystemsReady OnSystemsReady;

	/**
	 * Returns PlayerController at specified index.
	 * Results are cached for performance.
	 *
	 * @param playerIndex - Usually 0 for single-player
	 * @return PlayerController or nullptr
	 */
	UFUNCTION( BlueprintPure, Category = "Core|Utilities" )
	APlayerController* GetPlayerController( int32 playerIndex = 0 ) const;

	/**
	 * Logs status of all systems to output log.
	 * Useful for debugging initialization issues.
	 * Only runs in non-shipping builds.
	 */
	UFUNCTION( BlueprintCallable, Category = "Core|Debug" )
	void LogSystemsStatus() const;

protected:

	/**
	 * Generic manager registration with consistent logging and null checks.
	 *
	 * @tparam T - Manager type (must be UObject-derived)
	 * @param managerRef - Weak pointer to store reference
	 * @param manager - Manager instance to register
	 * @param managerName - Name for logging
	 */
	template <typename T>
	void RegisterManagerInternal( TWeakObjectPtr<T>& managerRef, T* manager, const TCHAR* managerName );

	/**
	 * Generic manager unregistration.
	 * Only clears if the provided manager matches stored reference.
	 */
	template <typename T>
	void UnregisterManagerInternal( TWeakObjectPtr<T>& managerRef, T* manager, const TCHAR* managerName );

	/** Weak reference to WaveManager actor in world. */
	UPROPERTY()
	TWeakObjectPtr<AWaveManager> WaveManager_;

	/** Weak reference to BuildManager actor in world. */
	UPROPERTY()
	TWeakObjectPtr<ABuildManager> BuildManager_;

	/** Weak reference to GridVisualizer actor in world. */
	UPROPERTY()
	TWeakObjectPtr<AGridVisualizer> GridVisualizer_;

	/** Weak reference to ResourceManager on PlayerController. */
	UPROPERTY()
	TWeakObjectPtr<UResourceManager> ResourceManager_;

	/** Weak reference to EconomyComponent on PlayerController. */
	UPROPERTY()
	TWeakObjectPtr<UEconomyComponent> EconomyComponent_;

	/** Weak reference to SelectionManager on PlayerController. Optional. */
	UPROPERTY()
	TWeakObjectPtr<USelectionManagerComponent> SelectionManager_;

	/**
	 * Strong reference to GameLoopManager.
	 * Owned by CoreManager - created and destroyed by us.
	 */
	UPROPERTY()
	TObjectPtr<UGameLoopManager> GameLoopManager_;

	/** Cached PlayerController to avoid repeated lookups. Mutable for const getters. */
	mutable TWeakObjectPtr<APlayerController> CachedPlayerController_;

	/** Index of cached PlayerController for validation. */
	mutable int32 CachedPlayerIndex_ = -1;

	/**
	 * Safely retrieves World pointer.
	 * Returns nullptr if GameInstance not available.
	 */
	UWorld* GetWorldSafe() const;

	/**
	 * Finds manager actors already placed in the world.
	 * Uses UGameplayStatics::GetActorOfClass for each type.
	 */
	void FindWorldActors();

	/**
	 * Finds or creates components on PlayerController.
	 * Creates ResourceManager and EconomyComponent if not found.
	 */
	void FindPlayerControllerComponents();

	/**
	 * Creates internally-owned managers (GameLoopManager).
	 */
	void CreateInternalManagers();

	/**
	 * Wires dependencies between managers.
	 * - EconomyComponent -> ResourceManager
	 * - GameLoopManager -> All required systems
	 */
	void SetupManagerConnections();

	/**
	 * Updates GameLoopManager's external dependencies.
	 * Called during RefreshSystemReferences().
	 */
	void UpdateGameLoopDependencies();

	/**
	 * Clears all cached references.
	 * Called during reset and deinitialization.
	 */
	void ClearAllReferences();

	/** True after InitializeSystems() completes. */
	bool bIsInitialized_ = false;
};
