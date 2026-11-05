// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Path/DStarLite.h"

#include "Core/CoreManager.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "Grid/GridManager.h"

#include "Kismet/GameplayStatics.h"

UDStarLite::UDStarLite()
{
}

void UDStarLite::Initialize( const FPathConfig& config )
{
	if ( const UCoreManager* core = UGameplayStatics::GetGameInstance( GetWorld() )->GetSubsystem<UCoreManager>() )
	{
		Grid_ = core->GetGridManager();
	}

	EmptyCellTravelTime_ = config.EmptyCellTravelTime;
	bIgnoreObstacles_ = config.bIgnoreObstacles;

	if ( config.UnitDamage > 0.0f && config.UnitCooldown > 0.0f )
	{
		UnitDps_ = config.UnitDamage / config.UnitCooldown;
	}
	else
	{
		UnitDps_ = -1.0f;
	}

	// Algorithm setup

	Nodes_.Empty();
	Open_ = std::priority_queue<FDStarQueueEntry, std::vector<FDStarQueueEntry>, DStarQueueEntryCompare>();
	Km_ = 0.0f;

	if ( !Grid_.IsValid() )
	{
		UE_LOG( LogTemp, Error, TEXT( "UDStarLite::Initialize: no valid Grid found. Cannot use pathfinding" ) );
		return;
	}

	Start_ = Grid_->FindClosestWalkableCellCoords( config.Start );
	Nodes_.FindOrAdd( Start_, FDStarNode( Start_ ) );

	Goal_ = Grid_->FindClosestWalkableCellCoords( config.Goal );
	FDStarNode& goalNode = Nodes_.FindOrAdd( Goal_, FDStarNode( Goal_ ) );
	goalNode.RHS = 0.0f;

	Open_.push( { goalNode.Coord, FDStarKey( Heuristic( Start_, goalNode.Coord ), 0.0f ) } );
	goalNode.bInOpen = true;

	bInitialized_ = true;
}

FDStarKey UDStarLite::CalculateKey( const FDStarNode& node ) const
{
	const float min = FMath::Min( node.G, node.RHS );
	return { min + Heuristic( Start_, node.Coord ) + Km_, min };
}

void UDStarLite::UpdateVertex( FDStarNode& node )
{
	if ( node.G != node.RHS )
	{
		Open_.push( { node.Coord, CalculateKey( node ) } );
		node.bInOpen = true;
	}
	else if ( node.G == node.RHS && node.bInOpen )
	{
		node.bInOpen = false; // lazy removal
	}
}

void UDStarLite::ComputeShortestPath()
{
	if ( !Grid_.IsValid() )
	{
		UE_LOG( LogTemp, Error, TEXT( "Trying to calculate path with invalid pointer to grid" ) );
		return;
	}
	if ( !bInitialized_ )
	{
		UE_LOG( LogTemp, Error, TEXT( "Trying to calculate path on uninitialized path" ) );
		return;
	}

	// Never keep a reference into Nodes_ across a FindOrAdd that may insert: adding an element
	// can reallocate the map storage and leave the reference dangling (undefined behaviour).
	// Likewise Open_.top() is copied, because push() may reallocate the queue.
	Nodes_.FindOrAdd( Start_, FDStarNode( Start_ ) );

	auto startNeedsUpdate = [this]()
	{
		const FDStarNode& startNode = Nodes_.FindChecked( Start_ );
		return Open_.top().Key < CalculateKey( startNode ) || startNode.RHS > startNode.G;
	};

	while ( !Open_.empty() && startNeedsUpdate() )
	{
		const FDStarQueueEntry top = Open_.top();
		const FIntPoint coord = top.NodeCoord;

		FDStarNode& node = Nodes_.FindOrAdd( coord, FDStarNode( coord ) );
		if ( !node.bInOpen )
		{
			Open_.pop(); // lazy removal
			continue;
		}

		const FDStarKey newKey = CalculateKey( node );
		if ( top.Key < newKey )
		{
			Open_.pop();
			Open_.push( { coord, newKey } );
			node.bInOpen = true;
		}
		else if ( node.G > node.RHS )
		{
			node.G = node.RHS;
			node.bInOpen = false;
			const float nodeG = node.G;
			Open_.pop();

			for ( const FIntPoint& pred : GetPredecessors( coord ) )
			{
				FDStarNode& predNode = Nodes_.FindOrAdd( pred, FDStarNode( pred ) );
				if ( pred != Goal_ )
				{
					predNode.RHS = FMath::Min( predNode.RHS, Cost( pred, coord ) + nodeG );
				}
				UpdateVertex( predNode );
			}
		}
		else
		{
			const float gOld = node.G;
			node.G = TNumericLimits<float>::Max();

			TArray<FIntPoint> nodes = GetPredecessors( coord );
			nodes.Add( coord );
			for ( const FIntPoint& pred : nodes )
			{
				Nodes_.FindOrAdd( pred, FDStarNode( pred ) );

				if ( pred != Goal_ && Nodes_.FindChecked( pred ).RHS == Cost( pred, coord ) + gOld )
				{
					float minValue = TNumericLimits<float>::Max();

					for ( const FIntPoint& succ : GetSuccessors( pred ) )
					{
						const float succG = Nodes_.FindOrAdd( succ, FDStarNode( succ ) ).G;
						minValue = FMath::Min( minValue, Cost( pred, succ ) + succG );
					}

					Nodes_.FindChecked( pred ).RHS = minValue;
				}
				UpdateVertex( Nodes_.FindChecked( pred ) );
			}
		}
	}
}

TArray<FIntPoint> UDStarLite::GetPath() const
{
	const FDStarNode* startNode = Nodes_.Find( Start_ );
	const FDStarNode* goalNode = Nodes_.Find( Goal_ );

	if ( !bInitialized_ || !startNode || !goalNode )
	{
		UE_LOG( LogTemp, Error, TEXT( "DStarLite: not initialized" ) );
		return {};
	}

	if ( FMath::IsNearlyEqual( startNode->G, TNumericLimits<float>::Max(), 1E-6 ) )
	{
		UE_LOG( LogTemp, Error, TEXT( "DStarLite: path not found" ) );
		return {};
	}

	TArray<FIntPoint> path;
	FIntPoint current = Start_;
	path.Add( current );

	int maxCount = Nodes_.Num() * 2;
	int count = 0;
	while ( current != Goal_ && count < maxCount )
	{
		float best = TNumericLimits<float>::Max();
		FIntPoint next = current;

		for ( const FIntPoint& succ : GetSuccessors( current ) )
		{
			const FDStarNode* succNode = Nodes_.Find( succ );
			if ( !succNode || succNode->G == TNumericLimits<float>::Max() )
			{
				continue;
			}

			const float value = Cost( current, succ ) + succNode->G;
			if ( value < best )
			{
				best = value;
				next = succ;
			}
		}

		if ( next == current || FMath::IsNearlyEqual( best, TNumericLimits<float>::Max(), 1E-6 ) )
		{
			UE_LOG( LogTemp, Error, TEXT( "DStarLite::GetPath: stuck while extracting path" ) );
			return {};
		}

		current = next;
		path.Add( current );

		count++;
	}

	if ( count == maxCount )
	{
		UE_LOG( LogTemp, Error, TEXT( "DStarLite::GetPath: failed (iteration limit reached)" ) );
		return {};
	}

	return path;
}

void UDStarLite::OnUpdateEdgeCost( const FIntPoint& from )
{
}

float UDStarLite::Heuristic( const FIntPoint& a, const FIntPoint& b ) const
{
	const float dx = FMath::Abs( a.X - b.X );
	const float dy = FMath::Abs( a.Y - b.Y );
	const float octileCells = ( dx + dy ) + ( 1.41421356f - 2.0f ) * FMath::Min( dx, dy );

	// Cost() is measured in seconds (EmptyCellTravelTime_ per cell), so the heuristic must be too.
	// In cells it overestimated the remaining cost (~3x) and produced non-optimal paths.
	return octileCells * FMath::Max( EmptyCellTravelTime_, 0.0f );
}

TArray<FIntPoint> UDStarLite::GetSuccessors( const FIntPoint& coord ) const
{
	if ( !Grid_.IsValid() )
	{
		return TArray<FIntPoint>();
	}

	static const FIntPoint directions[8] = { { 1, 0 }, { -1, 0 }, { 0, 1 },  { 0, -1 },
	                                         { 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 } };

	const int32 width = Grid_->GetGridWidth();
	const int32 height = Grid_->GetGridHeight();

	TArray<FIntPoint> successors;
	for ( const FIntPoint& dir : directions )
	{
		FIntPoint next = coord + dir;

		if ( next.X < 0 || next.X >= width || next.Y < 0 || next.Y >= height )
		{
			continue;
		}

		// Non-rectangular grids (#342) have no cell at some in-bounds coordinates
		const FGridCell* cell = Grid_->GetCell( next.X, next.Y );
		if ( !cell )
		{
			continue;
		}

		if ( !bIgnoreObstacles_ && !cell->bIsWalkable )
		{
			continue;
		}

		successors.Add( next );
	}

	return successors;
}

TArray<FIntPoint> UDStarLite::GetPredecessors( const FIntPoint& coord ) const
{
	// Equal to successors
	return GetSuccessors( coord );
}

float UDStarLite::Cost( const FIntPoint& a, const FIntPoint& b ) const
{
	const int dx = FMath::Abs( b.X - a.X );
	const int dy = FMath::Abs( b.Y - a.Y );

	if ( !Grid_.IsValid() )
	{
		return BIG_NUMBER;
	}

	float timeToDestroy = 0.0f;
	if ( const FGridCell* cell = Grid_->GetCell( b.X, b.Y ) )
	{
		const TWeakObjectPtr<ABuilding> occupant = cell->bIsBuildable && cell->bIsOccupied ? cell->Occupant : nullptr;
		if ( occupant.IsValid() && UnitDps_ != -1.0f )
		{
			timeToDestroy = occupant->Stats().Health() / UnitDps_;
		}
	}

	// Cardinal move
	if ( dx + dy == 1 )
	{
		return EmptyCellTravelTime_ + timeToDestroy;
	}

	// Diagonal move
	if ( dx == 1 && dy == 1 )
	{
		const FGridCell* cellX = Grid_->GetCell( a.X + ( b.X - a.X ), a.Y );
		const FGridCell* cellY = Grid_->GetCell( a.X, a.Y + ( b.Y - a.Y ) );

		const bool cellXBlocked = !cellX || ( !bIgnoreObstacles_ && ( !cellX->bIsWalkable || cellX->bIsOccupied ) );
		const bool cellYBlocked = !cellY || ( !bIgnoreObstacles_ && ( !cellY->bIsWalkable || cellY->bIsOccupied ) );

		if ( cellXBlocked || cellYBlocked )
		{
			return TNumericLimits<float>::Max();
		}

		static constexpr float sqrt2 = 1.41421356f;
		return sqrt2 * EmptyCellTravelTime_ + timeToDestroy;
	}

	return TNumericLimits<float>::Max();
}
