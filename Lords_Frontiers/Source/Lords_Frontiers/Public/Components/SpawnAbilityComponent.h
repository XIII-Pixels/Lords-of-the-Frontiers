// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "SpawnAbilityComponent.generated.h"

class AUnit;
class UUnitBuilder;

UCLASS( ClassGroup = ( Unit ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API USpawnAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	void GroupSpawnTick();
	void UnitSpawnTick();

	void SpawnUnit() const;

	void StopUnitMovement() const;
	void ResumeUnitMovement() const;

	FTransform FindValidTransform() const;

	UPROPERTY( EditDefaultsOnly, Category = "Settings" )
	TSubclassOf<AUnit> SpawnedClass_;

	UPROPERTY( EditDefaultsOnly, Category = "Settings", meta = ( ClampMin = 0 ) )
	int SpawnedCount_ = 2;

	UPROPERTY( EditDefaultsOnly, Category = "Settings", meta = ( ClampMin = 0, Units = "s" ) )
	float GroupSpawnInterval_ = 5.0f;

	UPROPERTY( EditDefaultsOnly, Category = "Settings", meta = ( ClampMin = 0, Units = "s" ) )
	float UnitSpawnInterval_ = 0.0f;

	UPROPERTY( EditDefaultsOnly, Category = "Settings" )
	bool StopWhileSpawning_ = true;

	UPROPERTY( EditDefaultsOnly, Category = "Settings", meta = ( ClampMin = 0, Units = "s" ) )
	float StopTimeBeforeSpawn_ = 0.0f;

	// UPROPERTY( EditDefaultsOnly, Category = "Settings" )
	// bool SpawnOnlyWhenSeesEnemy_ = false;

	UPROPERTY()
	TObjectPtr<UUnitBuilder> UnitSpawner_;

	FTimerHandle GroupSpawnTimer_;
	FTimerHandle UnitSpawnTimer_;
	FTimerHandle MovementTimer_;
};
