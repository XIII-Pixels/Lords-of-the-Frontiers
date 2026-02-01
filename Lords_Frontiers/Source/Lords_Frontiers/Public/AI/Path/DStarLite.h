// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include <queue>

#include "DStarLite.generated.h"

class AUnit;
class AGridManager;

struct FDStarKey
{
	float K1 = TNumericLimits<float>::Max();
	float K2 = TNumericLimits<float>::Max();

	bool operator<( const FDStarKey& other ) const
	{
		return ( K1 < other.K1 ) || ( K1 == other.K1 && K2 < other.K2 );
	}

	bool operator>( const FDStarKey& other ) const
	{
		return ( K1 > other.K1 ) || ( K1 == other.K1 && K2 > other.K2 );
	}
};

struct FDStarNode
{
	FIntPoint Coord;
	FDStarKey Key;

	float G = TNumericLimits<float>::Max();
	float RHS = TNumericLimits<float>::Max();

	bool bInOpen = false;

	FDStarNode( const FIntPoint& coord ) : Coord( coord )
	{
	}

	FDStarNode( const FIntPoint& coord, const FDStarKey& key ) : Coord( coord ), Key( key )
	{
	}

	bool operator<( const FDStarNode& other ) const
	{
		return other.Key < Key;
	}

	bool operator>( const FDStarNode& other ) const
	{
		return other.Key > Key;
	}
};

struct FDStarNodePtrCompare
{
	bool operator()( const FDStarNode* a, const FDStarNode* b ) const
	{
		return a->Key > b->Key;
	}
};


/** (Gregory-hub)
 * D*-lite algorithm implementation */
UCLASS()
class LORDS_FRONTIERS_API UDStarLite : public UObject
{
	GENERATED_BODY()

public:
	UDStarLite();

	void Initialize( const FIntPoint& start, const FIntPoint& goal );
	void SetGrid( TWeakObjectPtr<AGridManager> grid );
	void SetStart( const FIntPoint& newStart );
	void SetEmptyCellTravelTime( float emptyCellTravelTime );
	// Unit damage and cooldown are needed to calculate time to destroy a building
	void SetUnitAttackInfo( float damage, float cooldown );

	void ComputeShortestPath();

	void OnUpdateEdgeCost( const FIntPoint& from );

	TArray<FIntPoint> GetPath() const;

private:
	UPROPERTY()
	TWeakObjectPtr<AGridManager> Grid_;

	float UnitDps_ = -1.0f;
	float EmptyCellTravelTime_ = 1.0f;

	TMap<FIntPoint, FDStarNode> Nodes_;
	std::priority_queue<FDStarNode*, std::vector<FDStarNode*>, FDStarNodePtrCompare> Open_;

	FIntPoint Start_;
	FIntPoint Goal_;

	float Km_ = 0.0f;

	bool bInitialized = false;

	// Core methods

	void UpdateVertex( FDStarNode& node );
	FDStarKey CalculateKey( const FDStarNode& node ) const;

	TArray<FIntPoint> GetSuccessors( const FIntPoint& coord ) const;
	TArray<FIntPoint> GetPredecessors( const FIntPoint& coord ) const;

	float Cost( const FIntPoint& a, const FIntPoint& b ) const;
	float Heuristic( const FIntPoint& a, const FIntPoint& b ) const;
};
