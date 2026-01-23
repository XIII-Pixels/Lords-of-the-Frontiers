// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Path/DStarLite.h"

#include "Grid/GridManager.h"

UDStarLite::UDStarLite()
{
}

void UDStarLite::Initialize( const FIntPoint& start, const FIntPoint& goal )
{
	Start_ = start;
	Goal_ = goal;
	Km_ = 0.0f;

	Nodes_.Empty();
	Open_.Empty();

	FDStarNode& goalNode = Nodes_.FindOrAdd( Goal_, FDStarNode( Goal_ ) );
	goalNode.RHS = 0.0f;

	Open_.Push( { &goalNode, CalculateKey( goalNode ) } );
	goalNode.bInOpen = true;
}

FDStarKey UDStarLite::CalculateKey( const FDStarNode& node ) const
{
	float minGrhs = FMath::Min( node.G, node.RHS );
	return { minGrhs + Heuristic( Start_, node.Coord ) + Km_, minGrhs };
}

void UDStarLite::UpdateVertex( FDStarNode& node )
{
	if ( node.Coord != Goal_ )
	{
		float minRHS = BIG_NUMBER;

		for ( const FIntPoint& succ : GetSuccessors( node.Coord ) )
		{
			const FDStarNode& succNode = Nodes_.FindOrAdd( succ, FDStarNode( succ ) );
			minRHS = FMath::Min( minRHS, Cost( node.Coord, succ ) + succNode.G );
		}

		node.RHS = minRHS;
	}

	if ( node.bInOpen )
	{
		node.bInOpen = false;
	}

	if ( !FMath::IsNearlyEqual( node.G, node.RHS ) )
	{
		Open_.Push( { &node, CalculateKey( node ) } );
		node.bInOpen = true;
	}
}

bool UDStarLite::ComputeShortestPath()
{
	while ( !Open_.IsEmpty() )
	{
		const FDStarQueueNode top = Open_.Top();
		FDStarNode& startNode = Nodes_.FindOrAdd( Start_, FDStarNode( Start_ ) );

		if ( !( top.Key < CalculateKey( startNode ) ) && FMath::IsNearlyEqual( startNode.G, startNode.RHS ) )
		{
			return true;
		}

		Open_.Pop();

		FDStarNode& node = *top.Node;
		if ( node.G > node.RHS )
		{
			node.G = node.RHS;

			for ( const FIntPoint& pred : GetPredecessors( node.Coord ) )
			{
				UpdateVertex( Nodes_.FindOrAdd( pred, FDStarNode( pred ) ) );
			}
		}
		else
		{
			node.G = BIG_NUMBER;
			UpdateVertex( node );

			for ( const FIntPoint& pred : GetPredecessors( node.Coord ) )
			{
				UpdateVertex( Nodes_.FindOrAdd( pred, FDStarNode( pred ) ) );
			}
		}
	}

	return false;
}

void UDStarLite::OnUpdateEdgeCost( const FIntPoint& from )
{
	FDStarNode& fromNode = Nodes_.FindOrAdd( from, FDStarNode( from ) );
	UpdateVertex( fromNode );
}

void UDStarLite::SetStart( const FIntPoint& newStart )
{
	Km_ += Heuristic( Start_, newStart );
	Start_ = newStart;
}

void UDStarLite::SetEmptyCellTravelTime( float emptyCellTravelTime )
{
	EmptyCellTravelTime_ = emptyCellTravelTime;
}

void UDStarLite::SetUnitAttackInfo( float damage, float cooldown )
{
	if ( damage > 0.0f && cooldown > 0.0f )
	{
		UnitDps_ = damage / cooldown;
	}
	else
	{
		UnitDps_ = -1.0f;
	}
}

TArray<FIntPoint> UDStarLite::GetPath() const
{
	TArray<FIntPoint> path;
	FIntPoint current = Start_;

	path.Add( current );

	while ( current != Goal_ )
	{
		const FDStarNode* node = Nodes_.Find( current );
		if ( !node )
		{
			break;
		}

		float best = BIG_NUMBER;
		FIntPoint next = current;

		for ( const FIntPoint& succ : GetSuccessors( current ) )
		{
			const FDStarNode* succNode = Nodes_.Find( succ );
			if ( !succNode )
			{
				continue;
			}

			float value = Cost( current, succ ) + succNode->G;
			if ( value < best )
			{
				best = value;
				next = succ;
			}
		}

		if ( next == current )
		{
			break;
		}

		current = next;
		path.Add( current );
	}

	return path;
}

void UDStarLite::SetGrid( TWeakObjectPtr<AGridManager> grid )
{
	Grid_ = grid;
}

float UDStarLite::Heuristic( const FIntPoint& a, const FIntPoint& b ) const
{
	return FMath::Abs( a.X - b.X ) + FMath::Abs( a.Y - b.Y ); // Manhattan
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

		if ( !Grid_->GetCell( coord.X, coord.Y )->bIsBuildable )
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
		const TWeakObjectPtr<ABuilding> occupant = cell->Occupant;
		if ( occupant.IsValid() )
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
		static constexpr float sqrt2 = 1.41421356f;
		return sqrt2 * EmptyCellTravelTime_ + timeToDestroy;
	}

	return BIG_NUMBER;
}
