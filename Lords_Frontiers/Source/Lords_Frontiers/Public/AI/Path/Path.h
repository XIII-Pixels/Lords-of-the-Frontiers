// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "DStarLite.h"

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "Path.generated.h"

class AGridManager;
class AUnit;


/** (Gregory-hub)
 * Class that represents a path that is traveled by unit */
UCLASS( NotBlueprintable )
class LORDS_FRONTIERS_API UPath : public UObject
{
	GENERATED_BODY()

public:
	UPath();

	virtual void PostInitProperties() override;

	void Initialize( const FDStarLiteConfig& config );

	bool IsInitialized() const;

	// Update cell values. Path needs to be updated after calling this
	void OnUpdateCell( const FIntPoint& cell );

	// Calculate full path or update part of path (if grid has changed)
	void CalculateOrUpdate();

	const TArray<FIntPoint>& GetPoints() const;

private:
	UPROPERTY()
	TObjectPtr<UDStarLite> DStarLite_;

	TArray<FIntPoint> PathPoints_;
};
