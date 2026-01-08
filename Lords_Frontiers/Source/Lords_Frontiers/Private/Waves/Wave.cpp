#include "Lords_Frontiers/Public/Waves/Wave.h"

#include "Lords_Frontiers/Public/Waves/EnemyGroupSpawnPoint.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

FWave::FWave() : EnemyGroups(), SpawnPointReferences(), SpawnPointIds(), StartDelay( 0.0f ), GroupSpawnDelay( 0.0f )
{
}

FWave::FWave(
    const TArray<FEnemyGroup>& inGroups, const TArray<TSoftObjectPtr<AEnemyGroupSpawnPoint>>& inRefs,
    const TArray<FName>& inIds, float inStartDelay, float inGroupSpawnDelay
)
    : EnemyGroups( inGroups ), SpawnPointReferences( inRefs ), SpawnPointIds( inIds ),
      StartDelay( FMath::Max( 0.0f, inStartDelay ) ), GroupSpawnDelay( FMath::Max( 0.0f, inGroupSpawnDelay ) )
{
}

bool FWave::IsValid() const
{
	for ( const FEnemyGroup& enemyGroup : EnemyGroups )
	{
		if ( enemyGroup.IsValid() )
		{
			return true;
		}
	}
	return false;
}

int32 FWave::GetTotalEnemyCount() const
{
	int32 total = 0;
	for ( const FEnemyGroup& enemyGroup : EnemyGroups )
	{
		total += FMath::Max( 0, enemyGroup.Count );
	}
	return total;
}

int32 FWave::GetTotalValidGroups() const
{
	int32 countValid = 0;
	for ( const FEnemyGroup& enemyGroup : EnemyGroups )
	{
		if ( enemyGroup.IsValid() )
		{
			++countValid;
		}
	}
	return countValid;
}

float FWave::GetGroupStartTime( int32 groupIndex ) const
{
	const int32 clamped = FMath::Max( 0, groupIndex );
	return StartDelay + static_cast<float>( clamped ) * GroupSpawnDelay;
}

float FWave::GetGroupTotalDuration( int32 groupIndex ) const
{
	if ( groupIndex < 0 || groupIndex >= EnemyGroups.Num() )
	{
		return 0.0f;
	}
	return EnemyGroups[groupIndex].GetTotalSpawnDuration();
}

float FWave::GetTotalWaveDuration() const
{
	float endTime = StartDelay;
	const int32 numGroups = EnemyGroups.Num();

	for ( int32 i = 0; i < numGroups; ++i )
	{
		const float groupStart = GetGroupStartTime( i );
		const float groupDuration = EnemyGroups[i].GetTotalSpawnDuration();
		const float groupEnd = groupStart + groupDuration;

		if ( groupEnd > endTime )
		{
			endTime = groupEnd;
		}
	}

	return endTime;
}

float FWave::GetTimeToSpawnEnemy( int32 groupIndex, int32 enemyIndex ) const
{
	if ( groupIndex < 0 || groupIndex >= EnemyGroups.Num() )
	{
		return StartDelay;
	}

	const FEnemyGroup& enemyGroup = EnemyGroups[groupIndex];
	const float groupStart = GetGroupStartTime( groupIndex );

	return groupStart + enemyGroup.GetSpawnDelayForIndex( enemyIndex );
}

FName FWave::GetSpawnPointIdForGroup( int32 groupIndex ) const
{
	if ( SpawnPointIds.IsValidIndex( groupIndex ) )
	{
		return SpawnPointIds[groupIndex];
	}
	return NAME_None;
}

TSoftObjectPtr<AEnemyGroupSpawnPoint> FWave::GetSpawnPointRefForGroup( int32 groupIndex ) const
{
	if ( SpawnPointReferences.IsValidIndex( groupIndex ) )
	{
		return SpawnPointReferences[groupIndex];
	}
	return TSoftObjectPtr<AEnemyGroupSpawnPoint>();
}

FTransform FWave::GetSpawnTransformForGroup( UObject* worldContextObject, int32 groupIndex ) const
{
	if ( !worldContextObject )
	{
		return FTransform::Identity;
	}

	if ( SpawnPointIds.IsValidIndex( groupIndex ) )
	{
		const FName spawnPointId = SpawnPointIds[groupIndex];
		if ( !spawnPointId.IsNone() )
		{
			if ( AEnemyGroupSpawnPoint* foundSpawnPoint =
			         AEnemyGroupSpawnPoint::FindSpawnPointById( worldContextObject, spawnPointId ) )
			{
				return foundSpawnPoint->GetSpawnTransform();
			}
		}
	}

	return FTransform::Identity;
}
