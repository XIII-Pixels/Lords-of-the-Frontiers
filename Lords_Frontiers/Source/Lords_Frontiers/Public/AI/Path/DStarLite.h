// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include <queue>

#include "DStarLite.generated.h"

class AUnit;
class AGridManager;

/** (Gregory-hub)
 * Key for sorting node entries in priority queue */
USTRUCT()
struct FDStarKey
{
	GENERATED_BODY()

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

/** (Gregory-hub)
 * Represents grid cell */
USTRUCT()
struct FDStarNode
{
	GENERATED_BODY()

	FIntPoint Coord;

	float G = TNumericLimits<float>::Max();
	float RHS = TNumericLimits<float>::Max();

	bool bInOpen = false;

	FDStarNode() = default;

	FDStarNode( const FIntPoint& coord ) : Coord( coord )
	{
	}
};

/** (Gregory-hub)
 * Priority queue entry - node and key */
USTRUCT()
struct FDStarQueueEntry
{
	GENERATED_BODY()

	FIntPoint NodeCoord;
	FDStarKey Key;

	bool operator<( const FDStarQueueEntry& other ) const
	{
		return other.Key < Key;
	}

	bool operator>( const FDStarQueueEntry& other ) const
	{
		return other.Key > Key;
	}
};

/** (Gregory-hub)
 * Comparator class for priority queue sorting */
struct DStarQueueEntryCompare
{
	bool operator()( const FDStarQueueEntry& a, const FDStarQueueEntry& b ) const
	{
		return a.Key > b.Key;
	}
};

/** (Gregory-hub)
 * D*-lite algorithm configuration */
USTRUCT()
struct FDStarLiteConfig
{
	GENERATED_BODY()

	// Grid is required to calculate path
	TWeakObjectPtr<AGridManager> Grid;

	FIntPoint Start;
	FIntPoint Goal;

	// Unit damage and cooldown are needed to calculate time to destroy a building
	int UnitDamage;
	float UnitCooldown;

	float EmptyCellTravelTime;

	FDStarLiteConfig() = default;

	FDStarLiteConfig(
	    TWeakObjectPtr<AGridManager> grid, FIntPoint start, FIntPoint goal, int unitDamage, float unitCooldown,
	    float emptyCellTravelTime
	)
	    : Grid( grid ), Start( start ), Goal( goal ), UnitDamage( unitDamage ), UnitCooldown( unitCooldown ),
	      EmptyCellTravelTime( emptyCellTravelTime )
	{
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

	void Initialize( const FDStarLiteConfig& config );

	void OnUpdateEdgeCost( const FIntPoint& from );

	void ComputeShortestPath();

	TArray<FIntPoint> GetPath() const;

private:
	UPROPERTY()
	TWeakObjectPtr<AGridManager> Grid_;

	float UnitDps_ = -1.0f;
	float EmptyCellTravelTime_ = 1.0f;

	UPROPERTY()
	TMap<FIntPoint, FDStarNode> Nodes_;

	std::priority_queue<FDStarQueueEntry, std::vector<FDStarQueueEntry>, DStarQueueEntryCompare> Open_;

	FIntPoint Start_;
	FIntPoint Goal_;

	float Km_ = 0.0f;

	bool bInitialized_ = false;

	// Core methods

	void UpdateVertex( FDStarNode& node );
	FDStarKey CalculateKey( const FDStarNode& node ) const;

	TArray<FIntPoint> GetSuccessors( const FIntPoint& coord ) const;
	TArray<FIntPoint> GetPredecessors( const FIntPoint& coord ) const;

	float Cost( const FIntPoint& a, const FIntPoint& b ) const;
	float Heuristic( const FIntPoint& a, const FIntPoint& b ) const;
};
