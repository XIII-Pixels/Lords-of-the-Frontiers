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
		float minRHS = TNumericLimits<float>::Max();

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
			node.G = TNumericLimits<float>::Max();
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

void UDStarLite::SetEmptyCellCost( float emptyCellCost )
{
	EmptyCellCost_ = emptyCellCost;
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

		float best = TNumericLimits<float>::Max();
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

	if ( coord.X < 0 || coord.X >= Grid_->GetGridWidth() || coord.Y < 0 || coord.Y >= Grid_->GetGridHeight() )
	{
		return TArray<FIntPoint>();
	}

	// also need to check if cell is occupied
	TArray<FIntPoint> successors;
	if ( coord.X + 1 < Grid_->GetGridWidth() )
	{
		successors.Add( FIntPoint( coord.X + 1, coord.Y ) );
	}
	if ( coord.X - 1 >= 0 )
	{
		successors.Add( FIntPoint( coord.X - 1, coord.Y ) );
	}
	if ( coord.Y + 1 < Grid_->GetGridHeight() )
	{
		successors.Add( FIntPoint( coord.X, coord.Y + 1 ) );
	}
	if ( coord.Y - 1 >= 0 )
	{
		successors.Add( FIntPoint( coord.X, coord.Y - 1 ) );
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
	const FIntPoint diff( FMath::Abs( b.X - a.X ), FMath::Abs( b.Y - a.Y ) );
	if ( diff.X > 1 || diff.Y > 1 )
	{
		return TNumericLimits<float>::Max();
	}
	return diff.Size(); // + time to destroy building on cell
}
