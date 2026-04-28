// Fill out your copyright notice in the Description page of Project Settings.
#include "AI/Path/PathPointsManager.h"

#include "AI/Path/Path.h"
#include "AI/Path/PathTargetPoint.h"
#include "AI/UnitAIManager.h"
#include "Core/CoreManager.h"
#include "Grid/GridManager.h"

#include "Kismet/GameplayStatics.h"

void UPathPointsManager::GetAccessToGrid()
{
	if ( const UCoreManager* core = UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<UCoreManager>() )
	{
		Grid_ = core->GetGridManager();
	}
}

void UPathPointsManager::CreateAndRegisterPathPoints( const UPath& path, TSubclassOf<AUnit> unitClass )
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

	TSubclassOf<APathTargetPoint> pathPointClass = nullptr;
	if ( const UCoreManager* cm = UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<UCoreManager>() )
	{
		if ( const AUnitAIManager* unitAIManager = cm->GetUnitAIManager() )
		{
			pathPointClass = unitAIManager->GetPathPointClass( unitClass );
		}
	}

	if ( !pathPointClass )
	{
		UE_LOG(
		    LogTemp, Error, TEXT( "PathPointsManager: PathTargetPointClass not specified. Cannot add path targets" )
		);
		return;
	}

	for ( const FIntPoint& point : path.GetPoints() )
	{
		TWeakObjectPtr<APathTargetPoint> targetPoint = GetTargetPoint( point, unitClass );
		if ( targetPoint.IsValid() )
		{
			targetPoint->IncreaseRefCount();
		}
		else
		{
			TWeakObjectPtr<APathTargetPoint> pathPoint = SpawnPoint( point, pathPointClass );
			if ( pathPoint.IsValid() )
			{
				RegisterPoint( point, pathPoint.Get(), unitClass );
				bPointsVisible_ ? ShowPoint( pathPoint.Get() ) : HidePoint( pathPoint.Get() );
			}
		}
	}
}

void UPathPointsManager::RegisterPoint(
    const FIntPoint& point, APathTargetPoint* pathPoint, TSubclassOf<AUnit> unitClass
)
{
	if ( pathPoint )
	{
		FPointsOnCell& pointsOnCell = PathPoints_.FindOrAdd( point );
		if ( pathPoint->CreateIndependently() )
		{
			// Only use unitClass if point->CreateIndependently()
			pointsOnCell.Points().Add( unitClass, pathPoint );
		}
		else
		{
			pointsOnCell.Points().Add( AUnit::StaticClass(), pathPoint );
		}

		PathPoints_.Add( point, pointsOnCell );
		pathPoint->IncreaseRefCount();
	}
}

TWeakObjectPtr<APathTargetPoint>
UPathPointsManager::GetTargetPoint( const FIntPoint& point, TSubclassOf<AUnit> unitClass, bool notIndependent ) const
{
	if ( const FPointsOnCell* found = PathPoints_.Find( point ) )
	{
		if ( unitClass && found->Points().Contains( unitClass ) && found->Points()[unitClass]->CreateIndependently() &&
		     !notIndependent )
		{
			// Only use unitClass if point->CreateIndependently()
			return found->Points()[unitClass];
		}
		if ( found->Points().Contains( AUnit::StaticClass() ) )
		{
			return found->Points()[AUnit::StaticClass()];
		}
	}
	return nullptr;
}

void UPathPointsManager::ReleasePathPoint( const FIntPoint& point, TSubclassOf<AUnit> unitClass )
{
	const TWeakObjectPtr<APathTargetPoint> pathPoint = GetTargetPoint( point, unitClass );
	if ( pathPoint.IsValid() )
	{
		pathPoint->DecreaseRefCount();
		if ( pathPoint->RefCount() <= 0 )
		{
			pathPoint->Destroy();
			if ( FPointsOnCell* cell = PathPoints_.Find( point ) )
			{
				if ( unitClass && cell->Points().Contains( unitClass ) )
				{
					cell->Points().Remove( unitClass );
				}
				else
				{
					cell->Points().Remove( AUnit::StaticClass() );
				}

				if ( cell->Points().Num() == 0 )
				{
					PathPoints_.Remove( point );
				}
			}
		}
	}
}

void UPathPointsManager::ReleasePath( const UPath* path, TSubclassOf<AUnit> unitClass )
{
	if ( !path )
	{
		return;
	}

	for ( const FIntPoint& point : path->GetPoints() )
	{
		ReleasePathPoint( point, unitClass );
	}
}

void UPathPointsManager::Empty()
{
	for ( auto [point, pointsOnCell] : PathPoints_ )
	{
		for ( auto [unitClass, pathPoint] : pointsOnCell.Points() )
		{
			if ( IsValid( pathPoint ) )
			{
				pathPoint->Destroy();
			}
		}
	}
	PathPoints_.Empty();
	bPointsVisible_ = false;
}

void UPathPointsManager::ShowAll()
{
	for ( auto [point, pointsOnCell] : PathPoints_ )
	{
		for ( auto [unitClass, pathPoint] : pointsOnCell.Points() )
		{
			if ( IsValid( pathPoint ) )
			{
				ShowPoint( pathPoint );
			}
		}
	}

	bPointsVisible_ = true;
}

void UPathPointsManager::HideAll()
{
	for ( auto [point, pointsOnCell] : PathPoints_ )
	{
		for ( auto [unitClass, pathPoint] : pointsOnCell.Points() )
		{
			if ( IsValid( pathPoint ) )
			{
				HidePoint( pathPoint );
			}
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

TWeakObjectPtr<APathTargetPoint>
UPathPointsManager::SpawnPoint( const FIntPoint& point, TSubclassOf<APathTargetPoint> pathPointClass ) const
{
	FVector location;
	if ( !Grid_->GetCellWorldCenter( point, location ) )
	{
		UE_LOG(
		    LogTemp, Warning, TEXT( "PathPointsManager: failed to get world center for cell=[%d, %d]" ), point.Y,
		    point.X
		);
		return nullptr;
	}
	FRotator rotation( 0.0f, 0.0f, 0.0f );
	FTransform transform( rotation, location );

	FActorSpawnParameters spawnInfo;
	spawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if ( UWorld* world = GetWorld() )
	{
		return world->SpawnActor<APathTargetPoint>( pathPointClass, transform, spawnInfo );
	}
	return nullptr;
}
