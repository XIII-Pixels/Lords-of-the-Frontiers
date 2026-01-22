// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "DStarLite.generated.h"

class AGridManager;

struct FDStarNode
{
	FIntPoint Coord;

	float G = TNumericLimits<float>::Max();
	float RHS = TNumericLimits<float>::Max();

	bool bInOpen = false;

	FDStarNode() = default;
	explicit FDStarNode( const FIntPoint inCoord ) : Coord( inCoord )
	{
	}
};

struct FDStarKey
{
	float K1;
	float K2;

	bool operator<( const FDStarKey& other ) const
	{
		return ( K1 < other.K1 ) || ( K1 == other.K1 && K2 < other.K2 );
	}
};

struct FDStarQueueNode
{
	FDStarNode* Node;
	FDStarKey Key;

	bool operator<( const FDStarQueueNode& other ) const
	{
		return other.Key < Key;
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
	// Should be (cell width) / (unit speed) = (time to travel through cell)
	void SetEmptyCellCost( float emptyCellCost );

	bool ComputeShortestPath();

	void OnUpdateEdgeCost( const FIntPoint& from );

	TArray<FIntPoint> GetPath() const;

private:
	TWeakObjectPtr<AGridManager> Grid_;

	TMap<FIntPoint, FDStarNode> Nodes_;
	TArray<FDStarQueueNode> Open_;

	FIntPoint Start_;
	FIntPoint Goal_;

	float Km_ = 0.0f;
	float EmptyCellCost_ = 100.0f;

	// Core methods

	void UpdateVertex( FDStarNode& node );
	FDStarKey CalculateKey( const FDStarNode& node ) const;

	TArray<FIntPoint> GetSuccessors( const FIntPoint& coord ) const;
	TArray<FIntPoint> GetPredecessors( const FIntPoint& coord ) const;

	float Cost( const FIntPoint& a, const FIntPoint& b ) const;
	float Heuristic( const FIntPoint& a, const FIntPoint& b ) const;
};
