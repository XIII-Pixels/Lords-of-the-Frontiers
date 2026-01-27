#include "Core/GameLoopConfig.h"

UGameLoopConfig::UGameLoopConfig()
{
	StartingResources = FResourceReward( 100, 50, 10 );
	BaseCombatReward = FResourceReward( 50, 25, 0 );
	PerfectWaveBonus = FResourceReward( 25, 10, 0 );
}

FResourceReward UGameLoopConfig::GetScaledCombatReward( int32 waveNumber, bool bPerfect ) const
{
	const int32 validWaveNumber = FMath::Max( 1, waveNumber );

	const float multiplier = 1.0f + ( ( validWaveNumber - 1 ) * WaveRewardScaling );

	FResourceReward scaled = BaseCombatReward * multiplier;

	if ( bPerfect )
	{
		scaled += PerfectWaveBonus;
	}

	return scaled;
}
