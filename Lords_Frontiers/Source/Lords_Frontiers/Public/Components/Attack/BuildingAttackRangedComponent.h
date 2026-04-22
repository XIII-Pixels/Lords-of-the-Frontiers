// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/Attack/AttackRangedComponent.h"
#include "CoreMinimal.h"

#include "BuildingAttackRangedComponent.generated.h"

class AUnit;

UCLASS( meta = ( BlueprintSpawnableComponent ), ClassGroup = ( Attack ) )
class LORDS_FRONTIERS_API UBuildingAttackRangedComponent : public UAttackRangedComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	virtual void SetAttackMode() override;

	virtual bool EnemyIsValid( const AActor* enemyActor ) const override;

	UPROPERTY( EditDefaultsOnly, Category = "Settings", meta = ( AllowAbstract = "true" ) )
	TSet<TSubclassOf<AUnit>> IgnoredUnits_;
};
