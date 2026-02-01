// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Path/DStarLite.h"

#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "Grid/GridManager.h"

UDStarLite::UDStarLite()
{
}

FDStarKey UDStarLite::CalculateKey( const FDStarNode& node ) const
{
	const float min = FMath::Min( node.G, node.RHS );
	return { min + Heuristic( Start_, node.Coord ) + Km_, min };
}

void UDStarLite::Initialize( const FIntPoint& start, const FIntPoint& goal )
{
	Nodes_.Empty();
	Open_ = std::priority_queue<FDStarQueueEntry, std::vector<FDStarQueueEntry>, DStarQueueEntryCompare>();

	Km_ = 0.0f;

	Start_ = start;
	Nodes_.FindOrAdd( Start_, FDStarNode( Start_ ) );

	Goal_ = goal;
	FDStarNode& goalNode = Nodes_.FindOrAdd( Goal_, FDStarNode( Goal_ ) );
	goalNode.RHS = 0.0f;

	Open_.push( { goalNode.Coord, FDStarKey( Heuristic( Start_, goalNode.Coord ), 0.0f ) } );
	goalNode.bInOpen = true;

	bInitialized = true;
}

void UDStarLite::Reset()
{
	Initialize( Start_, Goal_ );
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
	FDStarNode& startNode = Nodes_.FindOrAdd( Start_, FDStarNode( Start_ ) );
	while ( !Open_.empty() && ( Open_.top().Key < CalculateKey( startNode ) || startNode.RHS > startNode.G ) )
	{
		const FDStarQueueEntry& top = Open_.top();
		FDStarNode& node = Nodes_.FindOrAdd( top.NodeCoord );
		if ( !node.bInOpen )
		{
			Open_.pop(); // lazy removal
			continue;
		}

		const FDStarKey& key = top.Key;

		FDStarKey newKey = CalculateKey( node );
		if ( key < newKey )
		{
			Open_.pop();
			Open_.push( { node.Coord, newKey } );
			node.bInOpen = true;
		}
		else if ( node.G > node.RHS )
		{
			node.G = node.RHS;
			node.bInOpen = false;
			Open_.pop();

			for ( const FIntPoint& pred : GetPredecessors( node.Coord ) )
			{
				FDStarNode& predNode = Nodes_.FindOrAdd( pred, FDStarNode( pred ) );
				if ( pred != Goal_ )
				{
					predNode.RHS = FMath::Min( predNode.RHS, Cost( predNode.Coord, node.Coord ) + node.G );
				}
				UpdateVertex( predNode );
			}
		}
		else
		{
			float gOld = node.G;
			node.G = TNumericLimits<float>::Max();
			TArray<FIntPoint> nodes = GetPredecessors( node.Coord );
			nodes.Add( node.Coord );
			for ( const FIntPoint& pred : nodes )
			{
				FDStarNode& predNode = Nodes_.FindOrAdd( pred, FDStarNode( pred ) );

				if ( predNode.RHS == Cost( pred, node.Coord ) + gOld && pred != Goal_ )
				{
					float minValue = TNumericLimits<float>::Max();

					for ( const FIntPoint& succ : GetSuccessors( pred ) )
					{
						FDStarNode& succNode = Nodes_.FindOrAdd( succ, FDStarNode( succ ) );
						minValue = std::min( minValue, Cost( pred, succ ) + succNode.G );
					}

					predNode.RHS = minValue;
				}
				UpdateVertex( predNode );
			}
		}
	}
}

TArray<FIntPoint> UDStarLite::GetPath() const
{
	const FDStarNode* startNode = Nodes_.Find( Start_ );
	const FDStarNode* goalNode = Nodes_.Find( Goal_ );

	if ( !bInitialized || !startNode || !goalNode )
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

void UDStarLite::SetStart( const FIntPoint& newStart )
{
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

void UDStarLite::SetGrid( TWeakObjectPtr<AGridManager> grid )
{
	Grid_ = grid;
}

float UDStarLite::Heuristic( const FIntPoint& a, const FIntPoint& b ) const
{
	float dx = abs( a.X - b.X );
	float dy = abs( a.Y - b.Y );
	return ( dx + dy ) + ( 1.41421356f - 2.f ) * FMath::Min( dx, dy );

	// return FMath::Abs( a.X - b.X ) + FMath::Abs( a.Y - b.Y ); // Manhattan
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

		if ( !Grid_->GetCell( next.X, next.Y )->bIsBuildable )
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
		static constexpr float sqrt2 = 1.41421356f;
		return sqrt2 * EmptyCellTravelTime_ + timeToDestroy;
	}

	return TNumericLimits<float>::Max();
}
