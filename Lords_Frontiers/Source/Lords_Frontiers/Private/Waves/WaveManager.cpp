#include "Lords_Frontiers/Public/Waves/WaveManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "NavigationSystem.h"      
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Math/UnrealMathUtility.h"

AWaveManager::AWaveManager () 
{
	PrimaryActorTick.bCanEverTick = false;
	CurrentWaveIndex = 0;
	bAutoStartOnBeginPlay = false;
	bLogSpawning = true;
	bIsWaveActive = false;
}

void AWaveManager::BeginPlay () 
{
	Super::BeginPlay ();

	if (  bAutoStartOnBeginPlay ) 
	{
		StartWaves ();
	}
}

int32 AWaveManager::ClampWaveIndex ( int32 waveIndex )  const
{
	if ( Waves.Num ()  == 0 ) 
	{
		return INDEX_NONE;
	}
	return FMath::Clamp ( waveIndex, 0, Waves.Num () - 1 );
}

void AWaveManager::StartWaves () 
{
	int32 startIndex = ClampWaveIndex ( CurrentWaveIndex );
	if ( startIndex == INDEX_NONE ) 
	{
		UE_LOG ( LogTemp, Warning, TEXT ( "WaveManager: No waves to start." ) );
		return;
	}

	StartWaveAtIndex ( startIndex );
	bHasRequestedFirstWave = true;
}

void AWaveManager::StartWaveAtIndex ( int32 waveIndex ) 
{
	int32 clampedIndex = ClampWaveIndex ( waveIndex );

	bHasRequestedFirstWave = true;

	if ( clampedIndex == INDEX_NONE ) 
	{
		UE_LOG ( LogTemp, Warning, TEXT ( "WaveManager: StartWaveAtIndex invalid index %d" ), waveIndex );
		return;
	}

	// If a wave is active, cancel it first (  safe behavior ) 
	if ( bIsWaveActive ) 
	{
		CancelCurrentWave ();
	}

	// Update current index
	CurrentWaveIndex = clampedIndex;
	const FWave& wave = Waves[CurrentWaveIndex];

	if ( !wave.IsValid () ) 
	{
		UE_LOG ( LogTemp, Warning, TEXT ( "WaveManager: Wave %d is not valid (  no enemy groups ) . Skipping." ), CurrentWaveIndex );

		MoveToNextWaveAndStart ();
		return;
	}

	// Broadcast start
	bIsWaveActive = true;
	OnWaveStarted.Broadcast ( CurrentWaveIndex );

	if ( bLogSpawning ) 
	{
		UE_LOG ( LogTemp,Log, TEXT ( "WaveManager: Starting wave %d (  StartDelay = %f ) " ), CurrentWaveIndex, wave.StartDelay );
	}

	// Schedule group of enemies spawns
	ScheduleWaveSpawns ( wave, CurrentWaveIndex );

	// Schedule wave end event
	const float totalDuration = wave.GetTotalWaveDuration ();
	const float endDelay = FMath::Max ( 0.0f, totalDuration );

	if ( GetWorld () ) 
	{
		GetWorld () -> GetTimerManager () .ClearTimer ( WaveEndTimerHandle );

		FTimerDelegate endDelegate;
		endDelegate.BindUFunction ( this, FName ( "OnWaveEndTimerElapsed" ), CurrentWaveIndex );

		GetWorld () -> GetTimerManager () .SetTimer ( WaveEndTimerHandle, endDelegate, endDelay, false );
	}
}

void AWaveManager::ScheduleWaveSpawns ( const FWave& wave, int32 waveIndex ) 
{
	if ( !GetWorld () ) 
	{
		return;
	}

	// Ensure prior timers cleared
	ClearActiveTimers ();

	const int32 numGroups = wave.EnemyGroups.Num ();
	for ( int32 groupIndex = 0; groupIndex < numGroups; ++groupIndex ) 
	{
		const FEnemyGroup& enemyGroup = wave.EnemyGroups[groupIndex];
		if ( !enemyGroup.IsValid () ) 
		{
			continue;
		}

		for ( int32 enemyIndex = 0; enemyIndex < enemyGroup.Count; ++enemyIndex ) 
		{
			const float timeFromWaveStart = wave.GetTimeToSpawnEnemy ( groupIndex, enemyIndex );

			if ( timeFromWaveStart < 0.f ) 
			{
				continue;
			}

			FTimerDelegate spawnDelegate;
			spawnDelegate.BindUFunction ( this, FName ( "SpawnEnemy" ), waveIndex, groupIndex, enemyIndex );

			FTimerHandle timerHandle;
			GetWorld () -> GetTimerManager () .SetTimer ( timerHandle, spawnDelegate, timeFromWaveStart, false );

			ActiveSpawnTimers.Add ( timerHandle );

			if ( bLogSpawning ) 
			{
				UE_LOG ( LogTemp, Log, TEXT ( "WaveManager: Scheduled spawn Wave[%d] Group[%d] Enemy[%d] at +%f s" ), waveIndex, groupIndex, enemyIndex, timeFromWaveStart );
			}
		}
	}
}

void AWaveManager::SpawnEnemy ( int32 waveIndex, int32 groupIndex, int32 enemyIndex ) 
{
	if ( !GetWorld () ) 
	{
		return;
	}

	if ( !Waves.IsValidIndex ( waveIndex ) ) 
	{
		return;
	}

	const FWave& wave = Waves[waveIndex];

	if ( !wave.EnemyGroups.IsValidIndex ( groupIndex ) ) 
	{
		return;
	}

	const FEnemyGroup& enemyGroup = wave.EnemyGroups[groupIndex];
	if ( !enemyGroup.IsValid () ) 
	{
		return;
	}

	if ( enemyIndex < 0 || enemyIndex >= enemyGroup.Count ) 
	{
		return;
	}

	FTransform spawnTransform = wave.GetSpawnTransformForGroup ( this, groupIndex );

	UE_LOG ( LogTemp, Warning, TEXT("WaveManager: RESOLVED transform for Wave[%d] Group[%d] = %s" ), waveIndex, groupIndex, *spawnTransform.GetLocation ().ToString () );

#if WITH_EDITOR
	DrawDebugSphere ( GetWorld (), spawnTransform.GetLocation (), 50.f, 8, FColor::Blue, false, 6.f );
#endif

	if ( spawnTransform.Equals ( FTransform::Identity ) ) 
	{
		spawnTransform = GetActorTransform ();
	}

	// get capsule size from class default (for Blueprint-based pawns)
	float capsuleRadius = 34.f;
	float capsuleHalfHeight = 88.f;

	UClass* enemyClass = enemyGroup.EnemyClass.Get();
	if ( enemyClass )
	{
		if ( AUnit* defaultUnit = Cast <AUnit> ( enemyClass -> GetDefaultObject () ) )
		{
			if ( UCapsuleComponent* C = defaultUnit -> FindComponentByClass <UCapsuleComponent> () )
			{
				capsuleRadius = C -> GetUnscaledCapsuleRadius ();
				capsuleHalfHeight = C -> GetUnscaledCapsuleHalfHeight ();
			}
		}
	}

	FTransform FinalTransform = FindNonOverlappingSpawnTransform ( spawnTransform, capsuleRadius, capsuleHalfHeight, 400.f, 24, /*bProjectToNavMesh=*/ false );
	
	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	spawnParams.Owner = this;
	spawnParams.Instigator = GetInstigator ();

	AUnit* spawned = GetWorld () -> SpawnActor<AUnit> ( enemyClass, FinalTransform, spawnParams );

	if ( !spawned ) 
	{
		UE_LOG(LogTemp, Warning, TEXT ("WaveManager: Failed to spawn actor for Wave[%d] Group[%d]" ), waveIndex, groupIndex );
		return;
	}

	if ( bLogSpawning ) 
	{
		UE_LOG ( LogTemp, Warning, TEXT("WaveManager: Spawned %s at Wave[%d] Group[%d] Enemy[%d]" ), *spawned->GetName (), waveIndex, groupIndex, enemyIndex );
	}
}

void AWaveManager::OnWaveEndTimerElapsed ( int32 waveIndex ) 
{
	if ( waveIndex != CurrentWaveIndex ) 
	{
		return;
	}

	bIsWaveActive = false;
	ClearActiveTimers ();

	OnWaveEnded.Broadcast ( waveIndex );

	if ( bLogSpawning ) 
	{
		UE_LOG ( LogTemp, Log, TEXT ( "WaveManager: Wave %d ended." ), waveIndex );
	}

	if ( waveIndex >= Waves.Num ()  - 1 ) 
	{
		OnAllWavesCompleted.Broadcast ();
		if ( bLogSpawning ) 
		{
			UE_LOG ( LogTemp, Log, TEXT ( "WaveManager: All waves completed." ) );
		}
	}
}

bool AWaveManager::MoveToNextWaveAndStart () 
{
	const int32 nextIndex = CurrentWaveIndex + 1;
	if ( !Waves.IsValidIndex ( nextIndex ) ) 
	{
		OnAllWavesCompleted.Broadcast ();
		if ( bLogSpawning ) 
		{
			UE_LOG ( LogTemp, Log, TEXT ( "WaveManager: No more waves to start." ) );
		}
		return false;
	}

	StartWaveAtIndex ( nextIndex );
	return true;
}

void AWaveManager::AdvanceToNextWave () 
{
	if ( bIsWaveActive ) 
	{
		CancelCurrentWave ();
	}

	const int32 nextIndex = CurrentWaveIndex + 1;
	if ( !Waves.IsValidIndex ( nextIndex ) ) 
	{
		OnAllWavesCompleted.Broadcast ();
		if ( bLogSpawning ) 
		{
			UE_LOG ( LogTemp, Log, TEXT ( "WaveManager: AdvanceToNextWave called but no more waves." ) );
		}
		return;
	}

	StartWaveAtIndex ( nextIndex );
}

void AWaveManager::CancelCurrentWave () 
{
	ClearActiveTimers ();

	if ( GetWorld () ) 
	{
		GetWorld () ->GetTimerManager () .ClearTimer ( WaveEndTimerHandle );
	}

	bIsWaveActive = false;

	if ( bLogSpawning ) 
	{
		UE_LOG ( LogTemp, Log, TEXT ( "WaveManager: Current wave cancelled." ) );
	}
}

void AWaveManager::RestartWaves () 
{
	CancelCurrentWave ();
	CurrentWaveIndex = 0;
	StartWaves ();
}

void AWaveManager::ClearActiveTimers () 
{
	if ( !GetWorld () ) 
	{
		ActiveSpawnTimers.Reset ();
		return;
	}

	FTimerManager& timerManager = GetWorld () -> GetTimerManager ();
	for ( FTimerHandle& timerHandle : ActiveSpawnTimers ) 
	{
		if ( timerHandle.IsValid () ) 
		{
			timerManager.ClearTimer ( timerHandle );
		}
	}

	ActiveSpawnTimers.Reset ();

	if ( WaveEndTimerHandle.IsValid () ) 
	{
		timerManager.ClearTimer ( WaveEndTimerHandle );
		WaveEndTimerHandle.Invalidate ();
	}
}

//(Artyom)
//find location around enemy group spawnpoint if there is a unit
FTransform AWaveManager::FindNonOverlappingSpawnTransform ( const FTransform& desiredTransform, float capsuleRadius, float capsuleHalfHeight, float maxSearchRadius,
	int32 maxAttempts, bool bProjectToNavMesh ) const
{
	UWorld* world = GetWorld ();
	if ( !world )
	{
		return desiredTransform;
	}

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent <UNavigationSystemV1> ( world );

	const FVector origin = desiredTransform.GetLocation ();
	const FQuat rotation = desiredTransform.GetRotation ();

	// collision query params
	FCollisionQueryParams queryParams ( SCENE_QUERY_STAT ( SpawnOverlap ), false );
	queryParams.bFindInitialOverlaps = true;
	queryParams.AddIgnoredActor ( this );

	const ECollisionChannel channelToTest = ECC_Pawn; // check collisions with pawn

	// try origin first
	{
		FCollisionShape shape = FCollisionShape::MakeCapsule (capsuleRadius, capsuleHalfHeight);
		if ( !world -> OverlapAnyTestByChannel ( origin, FQuat::Identity, channelToTest, shape, queryParams ) )
		{
#if WITH_EDITOR
			DrawDebugSphere ( world, origin, capsuleRadius + 10.f, 8, FColor::Green, false, 3.0f );
#endif
			FTransform Result = desiredTransform;
			Result.SetLocation ( origin );
			return Result;
		}
	}

	// spiral sampling
	const float angleStep = FMath::DegreesToRadians ( 30.0f );
	const int32 samplesPerRing = FMath::Max ( 6, FMath::RoundToInt ( 2.f * PI / angleStep) );
	const float radiusStep = FMath::Max ( capsuleRadius * 2.0f, 50.f );

	int32 attempt = 0;
	while ( attempt < maxAttempts)
	{
		const int32 ring = attempt / samplesPerRing + 1;
		const int32 index = attempt % samplesPerRing;
		const float radius = FMath::Min ( radiusStep * ring, maxSearchRadius );
		const float angle = angleStep * index;

		const FVector localOffset = FVector ( FMath::Cos ( angle ) * radius, FMath::Sin ( angle ) * radius, 0.f );
		FVector candidate = origin + rotation.RotateVector ( localOffset );

		// project to navmesh
		if ( bProjectToNavMesh && NavSys )
		{
			FNavLocation NavLoc;
			if ( NavSys -> ProjectPointToNavigation ( candidate, NavLoc, FVector ( 100.f, 100.f, 300.f ) ) )
			{ 
				candidate = NavLoc.Location;
			}
		}

		FCollisionShape shape = FCollisionShape::MakeCapsule ( capsuleRadius, capsuleHalfHeight );
		bool bOverlaps = world -> OverlapAnyTestByChannel ( candidate , FQuat::Identity, channelToTest, shape, queryParams );

		if ( !bOverlaps )
		{
#if WITH_EDITOR
			DrawDebugSphere ( world, candidate, capsuleRadius + 10.f, 8, FColor::Green, false, 3.0f );
#endif
			FTransform result = desiredTransform;
			result.SetLocation ( candidate );
			return result;
		}

		++attempt;
	}

	// not found - return desired
	return desiredTransform;
}

