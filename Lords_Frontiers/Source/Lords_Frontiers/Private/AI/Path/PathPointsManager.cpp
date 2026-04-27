// Fill out your copyright notice in the Description page of Project Settings.
#include "AI/Path/PathPointsManager.h"

#include "AI/Path/Path.h"
#include "AI/Path/PathTargetPoint.h"
#include "Core/CoreManager.h"
#include "Grid/GridManager.h"

#include "Kismet/GameplayStatics.h"

void UPathPointsManager::PostInitProperties()
{
	Super::PostInitProperties();

	if ( !PathTargetPointClass_ )
	{
		PathTargetPointClass_ = APathTargetPoint::StaticClass();
	}
}

void UPathPointsManager::GetAccessToGrid()
{
	if ( const UCoreManager* core = UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<UCoreManager>() )
	{
		Grid_ = core->GetGridManager();
	}
}

void UPathPointsManager::RegisterPathPoints( const UPath& path )
{
	if ( !Grid_.IsValid() )
	{
		GetAccessToGrid();

		if ( !Grid_.IsValid() )
		{
			UE_LOG( LogTemp, Error, TEXT( "PathPointsManager: Grid_ is not valid. Cannot add path targets" ) );
			return;
		}
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
		if ( PathPoints_.Contains( point ) )
		{
			PathPoints_[point]->IncreaseRefCount();
		}
		else
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
				bPointsVisible_ ? ShowPoint( pathPoint ) : HidePoint( pathPoint );
				PathPoints_.Add( point, pathPoint );
				pathPoint->IncreaseRefCount();
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

void UPathPointsManager::ReleasePathPoint( const FIntPoint& point )
{
	if ( const TObjectPtr<APathTargetPoint>* found = PathPoints_.Find( point ) )
	{
		const TObjectPtr<APathTargetPoint> pathPoint = *found;
		if ( IsValid( pathPoint ) )
		{
			pathPoint->DecreaseRefCount();
			if ( pathPoint->RefCount() <= 0 )
			{
				pathPoint->Destroy();
				PathPoints_.Remove( point );
			}
		}
	}
}

void UPathPointsManager::ReleasePath( const UPath* path )
{
	if ( !path )
	{
		return;
	}

	for ( const FIntPoint& point : path->GetPoints() )
	{
		ReleasePathPoint( point );
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
	bPointsVisible_ = false;
}

void UPathPointsManager::ShowAll()
{
	for ( auto [_, pathPoint] : PathPoints_ )
	{
		if ( IsValid( pathPoint ) )
		{
			ShowPoint( pathPoint );
		}
	}

	bPointsVisible_ = true;
}

void UPathPointsManager::HideAll()
{
	for ( auto [_, pathPoint] : PathPoints_ )
	{
		if ( IsValid( pathPoint ) )
		{
			HidePoint( pathPoint );
		}
	}

	bPointsVisible_ = false;
}

void UPathPointsManager::ShowPoint( APathTargetPoint* point, bool buildingAware ) const
{
	if ( !IsValid( point ) )
	{
		return;
	}
	if ( buildingAware && Grid_.IsValid() )
	{
		const FIntPoint coords = Grid_->GetClosestCellCoords( point->GetActorLocation() );
		const FGridCell* cell = Grid_->GetCell( coords.X, coords.Y );
		if ( cell && cell->bIsOccupied )
		{
			// Do not show point on a cell with building
			return;
		}
	}
	point->Show();
}

void UPathPointsManager::HidePoint( APathTargetPoint* point ) const
{
	if ( !IsValid( point ) )
	{
		return;
	}
	point->Hide();
}

bool UPathPointsManager::ActorIsOnPath( const AActor* actor, const UPath* path ) const
{
	if ( !actor || !path )
	{
		return false;
	}

	const AGridManager* grid = nullptr;
	if ( const UCoreManager* core = UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<UCoreManager>() )
	{
		grid = core->GetGridManager();
	}
	if ( !grid )
	{
		UE_LOG( LogTemp, Error, TEXT( "UAttackRangedComponent::EnemyIsOnPath: grid not found" ) );
		return false;
	}

	const FIntPoint enemyCoords = grid->GetCellCoords( actor->GetActorLocation() );
	// Path points storage will probably be reimplemented so this may be optimized in future
	for ( const FIntPoint& point : path->GetPoints() )
	{
		if ( point == enemyCoords )
		{
			return true;
		}
	}
	return false;
}
