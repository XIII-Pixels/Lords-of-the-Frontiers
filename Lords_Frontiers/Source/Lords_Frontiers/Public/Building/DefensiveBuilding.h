// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Building/Building.h"
#include "Building/WallTypes.h"
#include "CoreMinimal.h"

#include "DefensiveBuilding.generated.h"

/**
 *
 */
UCLASS()
class LORDS_FRONTIERS_API ADefensiveBuilding : public ABuilding
{
	GENERATED_BODY()

  public:
	ADefensiveBuilding();
	virtual FString GetNameBuild() override;
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "Wall" )
	const FWallMeshSet& GetWallMeshes() const
	{
		return WallMeshSet_;
	}

  protected:
	virtual void BeginPlay() override;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Wall" )
	FWallMeshSet WallMeshSet_;

	// Attack range (for the tower's component)
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Defense" )
	float AttackRange_;

	// Attack damage (can be taken from FEntityStats, but often duplicated)
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Defense" )
	int32 DefenseDamage_;
};
// DefensiveBuilding.h
