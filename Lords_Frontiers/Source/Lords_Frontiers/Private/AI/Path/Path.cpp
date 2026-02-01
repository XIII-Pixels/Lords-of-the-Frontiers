// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Path/Path.h"

#include "Grid/GridManager.h"

UPath::UPath()
{
}

void UPath::PostInitProperties()
{
	Super::PostInitProperties();
	DStarLite_ = NewObject<UDStarLite>( this );
}

void UPath::Initialize( const FDStarLiteConfig& config )
{
	DStarLite_->Initialize( config );
}

void UPath::OnUpdateCell( const FIntPoint& cell )
{
	DStarLite_->OnUpdateEdgeCost( cell );
}

void UPath::CalculateOrUpdate()
{
	DStarLite_->ComputeShortestPath();
	PathPoints_ = DStarLite_->GetPath();
}

const TArray<FIntPoint>& UPath::GetPoints() const
{
	return PathPoints_;
}
