#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "GameLoopConfig.generated.h"

/**
 * FResourceReward
 *
 * Encapsulates resource rewards into a single structure.
 * Used throughout the game loop for starting resources, combat rewards, and bonuses.
 */
USTRUCT( BlueprintType )
struct FResourceReward
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Resources", meta = ( ClampMin = "0" ) )
	int32 Gold = 0;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Resources", meta = ( ClampMin = "0" ) )
	int32 Food = 0;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Resources", meta = ( ClampMin = "0" ) )
	int32 Population = 0;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Resources", meta = ( ClampMin = "0" ) )
	int32 Progress = 0;

	FResourceReward() = default;

	/**
	 * Constructs a reward with validated non-negative values.
	 * Negative inputs are clamped to zero to prevent exploit vectors.
	 */
	FResourceReward( int32 inGold, int32 inFood, int32 inPopulation = 0, int32 inProgress = 0 )
		: Gold( FMath::Max( 0, inGold ) )
		, Food( FMath::Max( 0, inFood ) )
		, Population( FMath::Max( 0, inPopulation ) )
		, Progress( FMath::Max( 0, inProgress ) )
	{
	}

	/** Returns true if all resource values are zero. */
	bool IsEmpty() const
	{
		return Gold == 0 && Food == 0 && Population == 0 && Progress == 0;
	}

	FResourceReward operator+( const FResourceReward& other ) const
	{
		return FResourceReward(
			Gold + other.Gold, Food + other.Food,
			Population + other.Population, Progress + other.Progress
		);
	}

	FResourceReward operator-( const FResourceReward& other ) const
	{
		return FResourceReward(
			Gold - other.Gold, Food - other.Food,
			Population - other.Population, Progress - other.Progress
		);
	}

	FResourceReward& operator+=( const FResourceReward& other )
	{
		Gold += other.Gold;
		Food += other.Food;
		Population += other.Population;
		Progress += other.Progress;
		return *this;
	}

	FResourceReward& operator-=( const FResourceReward& other )
	{
		Gold -= other.Gold;
		Food -= other.Food;
		Population -= other.Population;
		Progress -= other.Progress;
		return *this;
	}

	FResourceReward operator*( float multiplier ) const
	{
		return FResourceReward(
			FMath::RoundToInt( Gold * multiplier ),
			FMath::RoundToInt( Food * multiplier ),
			FMath::RoundToInt( Population * multiplier ),
			FMath::RoundToInt( Progress * multiplier )
		);
	}

	FResourceReward operator/( float divisor ) const
	{
		return FResourceReward(
			FMath::RoundToInt( Gold / divisor ),
			FMath::RoundToInt( Food / divisor ),
			FMath::RoundToInt( Population / divisor ),
			FMath::RoundToInt( Progress / divisor )
		);
	}
};

/**
 * UGameLoopConfig
 *
 * Data Asset containing all tunable parameters for the game loop.
 */
UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UGameLoopConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UGameLoopConfig();

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Start" )
	FResourceReward StartingResources;

	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "Settings|Build",
		meta = ( ClampMin = "1", ClampMax = "10" )
	)
	int32 BuildTurnsBeforeCombat = 2;

	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "Settings|Combat",
		meta = ( ClampMin = "1.0", ClampMax = "300.0" )
	)
	float CombatDuration = 30.0f;

	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "Settings|Combat",
		meta = ( ClampMin = "0.0", ClampMax = "10.0" )
	)
	float CombatStartDelay = 1.5f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Rewards" )
	FResourceReward BaseCombatReward;

	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "Settings|Rewards",
		meta = ( ClampMin = "0.0", ClampMax = "1.0" )
	)
	float WaveRewardScaling = 0.15f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Rewards" )
	FResourceReward PerfectWaveBonus;

	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "Settings|Victory",
		meta = ( ClampMin = "1", ClampMax = "100" )
	)
	int32 WavesToWin = 10;

	UFUNCTION( BlueprintPure, Category = "Config" )
	FResourceReward GetScaledCombatReward( int32 waveNumber, bool bPerfect = false ) const;
};
