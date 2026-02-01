#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "GameLoopConfig.generated.h"

/**
 * FResourceReward
 *
 * Encapsulates resource rewards (gold, food, population) into a single structure.
 * Used throughout the game loop for starting resources, combat rewards, and bonuses.
 *
 * Design rationale:
 * - Grouping related data prevents parameter explosion in function signatures
 * - Operator overloads enable intuitive reward calculations (scaling, combining)
 * - Immutable-friendly design with const methods for safe usage
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

	FResourceReward() = default;

	/**
	 * Constructs a reward with validated non-negative values.
	 * Negative inputs are clamped to zero to prevent exploit vectors.
	 */
	FResourceReward( int32 inGold, int32 inFood, int32 inPopulation = 0 )
	    : Gold( FMath::Max( 0, inGold ) ), Food( FMath::Max( 0, inFood ) ), Population( FMath::Max( 0, inPopulation ) )
	{
	}

	/** Returns true if all resource values are zero. Used to skip empty reward notifications. */
	bool IsEmpty() const
	{
		return Gold == 0 && Food == 0 && Population == 0;
	}

	/** Combines two rewards. Useful for stacking base reward + bonus. */
	FResourceReward operator+( const FResourceReward& other ) const
	{
		return FResourceReward( Gold + other.Gold, Food + other.Food, Population + other.Population );
	}
	/** Deducts two awards. Useful for subtracting the base reward and bonus. */
	FResourceReward operator-( const FResourceReward& other ) const
	{
		return FResourceReward( Gold - other.Gold, Food - other.Food, Population - other.Population );
	}

	/** In-place addition for accumulating rewards. */
	FResourceReward& operator+=( const FResourceReward& other )
	{
		Gold += other.Gold;
		Food += other.Food;
		Population += other.Population;
		return *this;
	}

	/** in-place subtraction for accumulating rewards. */
	FResourceReward& operator-=( const FResourceReward& other )
	{
		Gold -= other.Gold;
		Food -= other.Food;
		Population -= other.Population;
		return *this;
	}

	/**
	 * Scales reward by a multiplier. Used for wave-based reward progression.
	 * Values are rounded to nearest integer to avoid floating point accumulation errors.
	 */
	FResourceReward operator*( float multiplier ) const
	{
		return FResourceReward(
		    FMath::RoundToInt( Gold * multiplier ), FMath::RoundToInt( Food * multiplier ),
		    FMath::RoundToInt( Population * multiplier )
		);
	}

	/**
	 * Reduces the reward by the divisor. Used to calculate the reward based on the wave.
	 * Values are rounded to the nearest integer to avoid floating-point accumulation errors.
	 */
	FResourceReward operator/( float multiplier ) const
	{
		return FResourceReward(
		    FMath::RoundToInt( Gold / multiplier ), FMath::RoundToInt( Food / multiplier ),
		    FMath::RoundToInt( Population / multiplier )
		);
	}

};

/**
 * UGameLoopConfig
 *
 * Data Asset containing all tunable parameters for the game loop.
 * Allows designers to adjust game balance without code changes.
 *
 * Usage:
 * 1. Create in Editor: Right-click -> Miscellaneous -> Data Asset -> GameLoopConfig
 * 2. Configure values in Details panel
 * 3. Assign to GameMode or pass to GameLoopManager::SetConfig()
 *
 * Note: Changes take effect on next game start, not mid-session.
 */
UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UGameLoopConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UGameLoopConfig();

	/** Resources granted at game start. Sets initial economy baseline. */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Start" )
	FResourceReward StartingResources;

	/**
	 * Number of build turns before combat begins.
	 * Higher values give players more preparation time but slow pacing.
	 * Recommended: 2-3 for fast games, 4-5 for strategic depth.
	 */
	UPROPERTY(
	    EditAnywhere, BlueprintReadOnly, Category = "Settings|Build", meta = ( ClampMin = "1", ClampMax = "10" )
	)
	int32 BuildTurnsBeforeCombat = 2;

	/**
	 * Combat phase duration in seconds.
	 * Timer continues regardless of remaining enemies.
	 * If enemies are cleared early, player waits for timer.
	 */
	UPROPERTY(
	    EditAnywhere, BlueprintReadOnly, Category = "Settings|Combat", meta = ( ClampMin = "1.0", ClampMax = "300.0" )
	)
	float CombatDuration = 30.0f;

	/**
	 * Delay before first enemy spawns after combat phase begins.
	 * Gives player a moment to observe the battlefield.
	 */
	UPROPERTY(
	    EditAnywhere, BlueprintReadOnly, Category = "Settings|Combat", meta = ( ClampMin = "0.0", ClampMax = "10.0" )
	)
	float CombatStartDelay = 1.5f;

	/** Base reward granted after each combat. Scaled by wave number. */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Rewards" )
	FResourceReward BaseCombatReward;

	/**
	 * Multiplier increase per wave. Formula: reward = base * (1 + (wave-1) * scaling)
	 * At 0.15: Wave 1 = 100%, Wave 5 = 160%, Wave 10 = 235%
	 */
	UPROPERTY(
	    EditAnywhere, BlueprintReadOnly, Category = "Settings|Rewards", meta = ( ClampMin = "0.0", ClampMax = "1.0" )
	)
	float WaveRewardScaling = 0.15f;

	/** Bonus added to reward if base took no damage during wave. */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Rewards" )
	FResourceReward PerfectWaveBonus;

	/** Number of waves player must survive to win. */
	UPROPERTY(
	    EditAnywhere, BlueprintReadOnly, Category = "Settings|Victory", meta = ( ClampMin = "1", ClampMax = "100" )
	)
	int32 WavesToWin = 10;

	/**
	 * Calculates final combat reward with wave scaling and perfect bonus.
	 *
	 * @param waveNumber - Current wave (1-indexed). Clamped to minimum of 1.
	 * @param bPerfect - True if base took no damage this wave.
	 * @return Scaled reward ready to grant to player.
	 */
	UFUNCTION( BlueprintPure, Category = "Config" )
	FResourceReward GetScaledCombatReward( int32 waveNumber, bool bPerfect = false ) const;
};