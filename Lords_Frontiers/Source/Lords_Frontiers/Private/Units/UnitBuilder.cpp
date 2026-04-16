// Fill out your copyright notice in the Description page of Project Settings.

#include "Units/UnitBuilder.h"

#include "Units/Unit.h"
#include "Utilities/TraceChannelMappings.h"

#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

void UUnitBuilder::CreateNewUnit(
    const TSubclassOf<AUnit>& subclass, const FTransform& transform, AActor* owner, APawn* instigator,
    ESpawnActorCollisionHandlingMethod handlingMethod
)
{
	if ( !GetWorld() )
	{
		UE_LOG( LogTemp, Error, TEXT( "UUnitBuilder::CreateNewUnit: world not found" ) );
		return;
	}

	if ( Unit_ )
	{
		Unit_->Destroy();
		Unit_ = nullptr;
	}
	Unit_ = GetWorld()->SpawnActorDeferred<AUnit>( subclass, transform, owner, instigator, handlingMethod );
}

void UUnitBuilder::SetTransform( const FTransform& transform ) const
{
	if ( Unit_ )
	{
		Unit_->SetActorTransform( transform );
	}
}

void UUnitBuilder::SetStats( const FEntityStats& stats ) const
{
	if ( Unit_ )
	{
		Unit_->Stats() = stats;
	}
}

void UUnitBuilder::ApplyBuff( const FEnemyBuff* buff ) const
{
	if ( Unit_ )
	{
		Unit_->ChangeStats( buff );
	}
}

TWeakObjectPtr<AUnit> UUnitBuilder::SpawnUnitAndFinish()
{
	const auto unit = Unit_;
	if ( Unit_ )
	{
		Unit_->FinishSpawning( Unit_->GetTransform() );
	}
	Unit_ = nullptr;
	return unit;
}

FTransform UUnitBuilder::FindNonOverlappingSpawnTransform(
    const FTransform& desiredTransform, float capsuleRadius, float capsuleHalfHeight, float maxSearchRadius,
    int32 maxAttempts, bool bProjectToNavMesh
) const
{
	UWorld* world = GetWorld();
	if ( !world )
	{
		return desiredTransform;
	}

	UNavigationSystemV1* navSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>( world );

	const FVector origin = desiredTransform.GetLocation();
	const FQuat rotation = desiredTransform.GetRotation();

	// collision query params
	FCollisionQueryParams queryParams( SCENE_QUERY_STAT( SpawnOverlap ), false );
	queryParams.bFindInitialOverlaps = true;

	constexpr ECollisionChannel channelToTest = ECC_Entity;

	// try origin first
	{
		FCollisionShape shape = FCollisionShape::MakeCapsule( capsuleRadius, capsuleHalfHeight );
		if ( !world->OverlapAnyTestByChannel( origin, FQuat::Identity, channelToTest, shape, queryParams ) )
		{

#if WITH_EDITOR
			DrawDebugSphere( world, origin, capsuleRadius + 10.f, 8, FColor::Green, false, 3.0f );
#endif

			FTransform Result = desiredTransform;
			Result.SetLocation( origin );
			return Result;
		}
	}

	// spiral sampling
	constexpr float angleStep = FMath::DegreesToRadians( 30.0f );
	const int32 samplesPerRing = FMath::Max( 6, FMath::RoundToInt( 2.f * PI / angleStep ) );
	const float radiusStep = FMath::Max( capsuleRadius * 2.0f, 50.f );

	int32 attempt = 0;
	while ( attempt < maxAttempts )
	{
		const int32 ring = attempt / samplesPerRing + 1;
		const int32 index = attempt % samplesPerRing;
		const float radius = FMath::Min( radiusStep * ring, maxSearchRadius );
		const float angle = angleStep * index;

		const FVector localOffset = FVector( FMath::Cos( angle ) * radius, FMath::Sin( angle ) * radius, 0.f );
		FVector candidate = origin + rotation.RotateVector( localOffset );

		// project to navmesh
		if ( bProjectToNavMesh && navSys )
		{
			FNavLocation navLoc;
			if ( navSys->ProjectPointToNavigation( candidate, navLoc, FVector( 100.f, 100.f, 300.f ) ) )
			{
				candidate = navLoc.Location;
			}
		}

		FCollisionShape shape = FCollisionShape::MakeCapsule( capsuleRadius, capsuleHalfHeight );
		bool bOverlaps =
		    world->OverlapAnyTestByChannel( candidate, FQuat::Identity, channelToTest, shape, queryParams );

		if ( !bOverlaps )
		{

#if WITH_EDITOR
			DrawDebugSphere( world, candidate, capsuleRadius + 10.f, 8, FColor::Green, false, 3.0f );
#endif

			FTransform result = desiredTransform;
			result.SetLocation( candidate );
			return result;
		}

		++attempt;
	}

	// not found - return desired
	return desiredTransform;
}
