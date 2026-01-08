#pragma once

#include "EnemyGroup.h"
#include "EnemyGroupSpawnPoint.h"

#include "CoreMinimal.h"

#include "Wave.generated.h"

class AEnemyGroupSpawnPoint;

/*
 (Artyom)
 description of one wave(FWave)
 Supports SpawnPoint's as pointers (Soft Object Ptr) or by id (FName)

 inner logic ( when GetSpawnTransformForGroup called ):
 - try SpawnPointIds[GroupIndex] (if given),
 - else return Identity (calling object must choose default spawn place or spawn
 in other place)
 */
USTRUCT( BlueprintType )
struct FWave
{
	GENERATED_BODY()

	// UPROPERTIES
	// enemy groups in wave
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Wave" )
	TArray<FEnemyGroup> EnemyGroups;

	// (optional) refs on some actor-spawnpoints for groups
	// if element is empty or not valid � SpawnPointIds will be called
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Wave" )
	TArray<TSoftObjectPtr<AEnemyGroupSpawnPoint>> SpawnPointReferences;

	// (optional) identificators for spawnpoints (FName) for groups
	// if SpawnPointReferences gives no result � search AEnemyGroupSpawnPoint by
	// id
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Wave" )
	TArray<FName> SpawnPointIds;

	// delay before wave start (seconds)
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Wave", meta = ( ClampMin = "0.0" ) )
	float StartDelay = 0.0f;

	// delay between groups in wave (sec)
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Wave", meta = ( ClampMin = "0.0" ) )
	float GroupSpawnDelay = 0.0f;

	// CPP FUNCTIONS
	FWave();
	FWave(
	    const TArray<FEnemyGroup>& inGroups,
	    const TArray<TSoftObjectPtr<AEnemyGroupSpawnPoint>>& inRefs = TArray<TSoftObjectPtr<AEnemyGroupSpawnPoint>>(),
	    const TArray<FName>& inIds = TArray<FName>(), float inStartDelay = 0.0f, float inGroupSpawnDelay = 0.0f
	);

	// if there is at least one valid group
	bool IsValid() const;

	// total amount in every group
	int32 GetTotalEnemyCount() const;

	// total amount of groups (invalid included)
	int32 GetGroupCount() const
	{
		return EnemyGroups.Num();
	}

	// total amount of valid groups
	int32 GetTotalValidGroups() const;

	// get start time of group from wave start (StartDelay + index *
	// GroupSpawnDelay)
	float GetGroupStartTime( int32 groupIndex ) const;

	// rets group total duration (uses FEnemyGroup::GetTotalSpawnDuration)
	float GetGroupTotalDuration( int32 groupIndex ) const;

	float GetTotalWaveDuration() const;

	// time before specific enemy (GroupIndex, EnemyIndex) relative to the
	// beginning of wave.
	float GetTimeToSpawnEnemy( int32 groupIndex, int32 enemyIndex ) const;

	// spawn point resolution
	// This is a C++ method, use WaveBPLibrary to call it from blueprints
	FTransform GetSpawnTransformForGroup( UObject* worldContextObject, int32 groupIndex ) const;

	FName GetSpawnPointIdForGroup( int32 groupIndex ) const;
	TSoftObjectPtr<AEnemyGroupSpawnPoint> GetSpawnPointRefForGroup( int32 groupIndex ) const;
};
