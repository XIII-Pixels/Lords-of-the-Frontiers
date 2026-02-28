// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Attacker.h"
#include "Building/Building.h"
#include "Building/WallTypes.h"
#include "ControlledByTree.h"

#include "CoreMinimal.h"

#include "DefensiveBuilding.generated.h"

class UAttackComponent;
class AAIController;
class UBehaviorTree;

/**
 *
 */
UCLASS()
class LORDS_FRONTIERS_API ADefensiveBuilding : public ABuilding, public IAttacker, public IControlledByTree
{
	GENERATED_BODY()

public:
	ADefensiveBuilding();

	virtual void OnConstruction( const FTransform& transform ) override;

	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "Settings|Wall" )
	const FWallMeshSet& GetWallMeshes() const
	{
		return WallMeshSet_;
	}

	virtual void Attack( TObjectPtr<AActor> hitActor ) override;

	virtual TObjectPtr<UBehaviorTree> BehaviorTree() const override;

	virtual TObjectPtr<AActor> EnemyInSight() const override;

	virtual void RestoreFromRuins() override;

	virtual void FullRestore() override;

protected:
	virtual void BeginPlay() override;

	virtual void OnDeath() override;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Wall" )
	FWallMeshSet WallMeshSet_;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|AI" )
	TSubclassOf<AAIController> BuildingAIControllerClass_;

	UPROPERTY( EditDefaultsOnly, Category = "Settings|AI" )
	TObjectPtr<UBehaviorTree> BuildingBehaviorTree_;

	UPROPERTY()
	TObjectPtr<UAttackComponent> AttackComponent_;
};
// DefensiveBuilding.h
