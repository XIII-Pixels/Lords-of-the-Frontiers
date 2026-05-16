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

USTRUCT()
struct FInfinitePurchase
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UEnemyPresetData> Preset = nullptr;

	UPROPERTY()
	int32 Times = 0;

	UPROPERTY()
	FName ChosenSector = NAME_None;

	UPROPERTY()
	FName ChosenPortalId = NAME_None;
};

UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UInfiniteWaveBuilder : public UObject
{
	GENERATED_BODY()

public:
	void Initialize( UInfiniteModeConfig* config, int32 sessionSeed );

	void ResetState();

	UWaveData* BuildWave( int32 waveIndex, UObject* worldContextObject );

	UPROPERTY( Transient, BlueprintReadOnly )
	FName LastThemeId = NAME_None;

	UPROPERTY( Transient, BlueprintReadOnly )
	int32 LastBudget = 0;

	UPROPERTY( Transient, BlueprintReadOnly )
	FEnemyBuff LastScalingBuff;

private:
	UPROPERTY( Transient )
	TObjectPtr<UInfiniteModeConfig> Config = nullptr;

	int32 SessionSeed = 0;

	int32 CarryOverBudget = 0;
	int32 LastApexWave = -10000;
	TMap<FName, int32> LastWaveSeenPreset_;
	TMap<FName, int32> LastWaveSeenSector_;

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

	TArray<FName> CollectAllSectors( UObject* worldContextObject ) const;
	TArray<FName> PickActiveSectors(
	    FRandomStream& rng, const FInfiniteTheme& theme, int32 waveIndex,
	    const TArray<FName>& allSectors
	) const;
	void AssignPortalsToPurchases(
	    FRandomStream& rng, TArray<FInfinitePurchase>& purchases, const TArray<FName>& activeSectors,
	    const TMap<FName, TArray<FName>>& sectorToPortalIds, const TArray<FName>& flyerFreePortals
	);

	UWaveData* AssembleWaveData(
	    const TArray<FInfinitePurchase>& purchases, const FInfiniteTheme& theme, const FEnemyBuff& scalingBuff,
	    bool bSpike
	) const;

	float RecencyMultiplier( int32 lastSeenWave, int32 currentWave ) const;

	void RememberWave( int32 waveIndex, const TArray<FInfinitePurchase>& purchases, bool bApexSeen );
};
