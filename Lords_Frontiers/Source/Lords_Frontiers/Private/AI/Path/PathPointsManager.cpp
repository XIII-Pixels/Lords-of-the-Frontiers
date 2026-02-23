// Fill out your copyright notice in the Description page of Project Settings.
#include "AI/Path/PathPointsManager.h"

#include "AI/Path/Path.h"
#include "AI/Path/PathTargetPoint.h"
#include "Grid/GridManager.h"

void UPathPointsManager::PostInitProperties()
{
	Super::PostInitProperties();

	if ( !PathTargetPointClass_ )
	{
		PathTargetPointClass_ = APathTargetPoint::StaticClass();
	}
}

void UPathPointsManager::SetGrid( TWeakObjectPtr<AGridManager> grid )
{
	Grid_ = grid;
}

void UPathPointsManager::AddPathPoints( const UPath& path )
{
	if ( !Grid_.IsValid() )
	{
		UE_LOG( LogTemp, Error, TEXT( "PathPointsManager: Grid_ is not valid. Cannot add path targets" ) );
		return;
	}

	if ( !PathTargetPointClass_ )
	{
		UE_LOG(
		    LogTemp, Error, TEXT( "PathPointsManager: PathTargetPointClass not specified. Cannot add path targets" )
		);
		return;
	}

	for ( const FIntPoint& point : path.GetPoints() )
	{
		if ( !PathPoints_.Contains( point ) )
		{
			FVector location;
			if ( !Grid_->GetCellWorldCenter( point, location ) )
			{
				UE_LOG(
				    LogTemp, Warning, TEXT( "PathPointsManager: failed to get world center for cell=[%d, %d]" ),
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
				pathPoint = world->SpawnActor<APathTargetPoint>( PathTargetPointClass_, transform, spawnInfo );
			}

			if ( pathPoint )
			{
				PathPoints_.Add( point, pathPoint );
			}
		}
	}
}

TWeakObjectPtr<APathTargetPoint> UPathPointsManager::GetTargetPoint( const FIntPoint& point ) const
{
	if ( const TObjectPtr<APathTargetPoint>* found = PathPoints_.Find( point ) )
	{
		return found->Get();
	}
	else
	{
		return nullptr;
	}
}

void UPathPointsManager::Remove( const FIntPoint& point )
{
	if ( TObjectPtr<APathTargetPoint>* found = PathPoints_.Find( point ) )
	{
		const TObjectPtr<APathTargetPoint> pathPoint = *found;
		if ( IsValid( pathPoint ) )
		{
			pathPoint->Destroy();
		}

		PathPoints_.Remove( point );
	}
}

void UPathPointsManager::Empty()
{
	for ( auto [_, pathPoint] : PathPoints_ )
	{
		if ( IsValid( pathPoint ) )
		{
			pathPoint->Destroy();
		}
	}
	PathPoints_.Empty();
}

void UPathPointsManager::ShowAll()
{
	for ( auto [_, pathPoint] : PathPoints_ )
	{
		if ( IsValid( pathPoint ) )
		{
			pathPoint->Show();
		}
	}
}

void UPathPointsManager::HideAll()
{
	for ( auto [_, pathPoint] : PathPoints_ )
	{
		if ( IsValid( pathPoint ) )
		{
			pathPoint->Hide();
		}
	}
}
