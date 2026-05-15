#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "Lords_Frontiers/Public/Waves/EnemyBuff.h"
#include "Lords_Frontiers/Public/Waves/Infinite/EnemyPresetData.h"
#include "Lords_Frontiers/Public/Waves/Infinite/InfiniteModeConfig.h"
#include "Lords_Frontiers/Public/Waves/Infinite/InfiniteModeTypes.h"
#include "Lords_Frontiers/Public/Waves/WaveData.h"

#include "InfiniteWaveBuilder.generated.h"

class AEnemyGroupSpawnPoint;

/*
 * Snapshot of per-wave purchase: which preset, how many times bought.
 * Sector/portal are assigned later in PortalAssignment pass.
 */
USTRUCT()
struct FInfinitePurchase
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UEnemyPresetData> Preset = nullptr;

	UPROPERTY()
	int32 Times = 0;

	// Filled in the portal pass. May stay empty for flyer free-portals --
	// in that case ChosenPortalId is the only valid field.
	UPROPERTY()
	FName ChosenSector = NAME_None;

	UPROPERTY()
	FName ChosenPortalId = NAME_None;
};

/*
 * Builds a transient UWaveData for a given infinite-mode wave index.
 *
 * The builder is stateful across waves to maintain anti-repeat / cooldown
 * counters (carry-over budget, last-seen-wave per preset and per sector,
 * apex cooldown).
 */
UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UInfiniteWaveBuilder : public UObject
{
	GENERATED_BODY()

public:
	void Initialize( UInfiniteModeConfig* config, int32 sessionSeed );

	// Reset all stateful counters. Called on RestartWaves().
	void ResetState();

	// Build a transient UWaveData for a given absolute wave index.
	// Returns nullptr if config is missing / pool is empty.
	UWaveData* BuildWave( int32 waveIndex, UObject* worldContextObject );

	// Diagnostic: last theme rolled (for UI / logging).
	UPROPERTY( Transient, BlueprintReadOnly )
	FName LastThemeId = NAME_None;

	// Diagnostic: last computed budget (after spike multiplier, after
	// carry-over).
	UPROPERTY( Transient, BlueprintReadOnly )
	int32 LastBudget = 0;

	// Diagnostic: last scaling buff used.
	UPROPERTY( Transient, BlueprintReadOnly )
	FEnemyBuff LastScalingBuff;

private:
	UPROPERTY( Transient )
	TObjectPtr<UInfiniteModeConfig> Config = nullptr;

	int32 SessionSeed = 0;

	// State carried across waves.
	int32 CarryOverBudget = 0;
	int32 LastApexWave = -10000;
	TMap<FName, int32> LastWaveSeenPreset_;   // PresetAsset->GetFName() -> wave
	TMap<FName, int32> LastWaveSeenSector_;   // sector name -> wave

	// --- Algorithm helpers --------------------------------------------------
	int32 ComputeBudget( int32 waveIndex ) const;
	bool IsSpikeWave( int32 waveIndex ) const;
	FEnemyBuff ComputeScalingBuff( int32 waveIndex ) const;

	const FInfiniteTheme* RollTheme( FRandomStream& rng, int32 waveIndex, bool bSpike ) const;
	float PresetWeight(
	    const UEnemyPresetData* preset, const FInfiniteTheme& theme, int32 waveIndex, int32 remainingBudget,
	    const TMap<FName, int32>& usedThisWave, bool bCoreOnly
	) const;

	UEnemyPresetData* PickPreset(
	    FRandomStream& rng, const FInfiniteTheme& theme, int32 waveIndex, int32 remainingBudget,
	    const TMap<FName, int32>& usedThisWave, bool bCoreOnly, bool bOffThemeOnly
	) const;

	// Sector pass
	TArray<FName> CollectAllSectors( UObject* worldContextObject ) const;
	TArray<FName> PickActiveSectors(
	    FRandomStream& rng, const FInfiniteTheme& theme, int32 waveIndex,
	    const TArray<FName>& allSectors
	) const;
	void AssignPortalsToPurchases(
	    FRandomStream& rng, TArray<FInfinitePurchase>& purchases, const TArray<FName>& activeSectors,
	    const TMap<FName, TArray<FName>>& sectorToPortalIds, const TArray<FName>& flyerFreePortals
	);

	// Final assembly
	UWaveData* AssembleWaveData(
	    const TArray<FInfinitePurchase>& purchases, const FInfiniteTheme& theme, const FEnemyBuff& scalingBuff,
	    bool bSpike
	) const;

	// Anti-repeat helper
	float RecencyMultiplier( int32 lastSeenWave, int32 currentWave ) const;

	// Update memory after a wave is built.
	void RememberWave( int32 waveIndex, const TArray<FInfinitePurchase>& purchases, bool bApexSeen );
};
