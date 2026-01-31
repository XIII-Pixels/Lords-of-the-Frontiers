// Fill out your copyright notice in the Description page of Project Settings.
#include "AI/Path/Path.h"
#include "AI/Path/PathPointsManager.h"
#include "AI/Path/PathTargetPoint.h"
#include "Grid/GridManager.h"

void APathPointsManager::PostInitProperties()
{
	Super::PostInitProperties();

	if ( !PathTargetPointClass )
	{
		PathTargetPointClass = APathTargetPoint::StaticClass();
	}
}

void APathPointsManager::SetGrid( TWeakObjectPtr<AGridManager> grid )
{
	Grid = grid;
}

void APathPointsManager::AddPathPoints( const UPath& path )
{
	if ( !Grid.IsValid() )
	{
		UE_LOG( LogTemp, Error, TEXT( "UPathPointsManager: Grid_ is not valid. Cannot add path targets" ) );
		return;
	}

	if ( !PathTargetPointClass )
	{
		UE_LOG(
		    LogTemp, Error, TEXT( "UPathPointsManager: PathTargetPointClass not specified. Cannot add path targets" )
		);
		return;
	}

	for ( const FIntPoint& point : path.GetPoints() )
	{
		if ( !PathPoints_.Contains( point ) )
		{
			FVector location;
			if ( !Grid->GetCellWorldCenter( point, location ) )
			{
				UE_LOG(
				    LogTemp, Warning, TEXT( "UPathPointsManager: failed to get world center for cell=[%d, %d]" ),
				    point.Y, point.X
				);
				continue;
			}
			FRotator rotation( 0.0f, 0.0f, 0.0f );
			FTransform transform( rotation, location );

			FActorSpawnParameters spawnInfo;
			spawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			APathTargetPoint* pathPoint = nullptr;
			if ( UWorld* world = GetWorld() )
			{
				pathPoint = world->SpawnActor<APathTargetPoint>( PathTargetPointClass, transform, spawnInfo );
			}

			if ( pathPoint )
			{
				PathPoints_.Add( point, pathPoint );
			}
		}
	}
}

TWeakObjectPtr<AActor> APathPointsManager::GetTargetPoint( const FIntPoint& point ) const
{
	if ( const auto found = PathPoints_.Find( point ) )
	{
		return found->Get();
	}
	else
	{
		return nullptr;
	}
}

void APathPointsManager::Remove( const FIntPoint& point )
{
	if ( TObjectPtr<APathTargetPoint>* found = PathPoints_.Find( point ) )
	{
		if ( const TObjectPtr<APathTargetPoint> pathPoint = *found )
		{
			pathPoint->Destroy();
		}

		PathPoints_.Remove( point );
	}
}

void APathPointsManager::Empty()
{
	for ( auto [_, pathPoint] : PathPoints_ )
	{
		if ( pathPoint )
		{
			pathPoint->Destroy();
		}
	}
	PathPoints_.Empty();
}
