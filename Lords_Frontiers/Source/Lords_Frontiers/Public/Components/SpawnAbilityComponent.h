// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "SpawnAbilityComponent.generated.h"

class UNiagaraSystem;
class AUnit;
class UUnitBuilder;

UCLASS( ClassGroup = ( Unit ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API USpawnAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	float TimeUntilGroupSpawnStart() const;

	// Stops all pending spawns (called when the owner dies)
	void StopSpawning();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay( const EEndPlayReason::Type endPlayReason ) override;

	void GroupSpawnTick();
	void UnitSpawnTick();
	void SpawnWholeGroup();

	void ProcessSpawn() const;

	void SpawnUnit() const;

	void StopUnitMovementAndAttack() const;
	void ResumeUnitMovementAndAttack() const;

	FTransform FindValidTransform() const;

	void ResolveVFXDefaults();

	UPROPERTY( EditDefaultsOnly, Category = "Settings" )
	TSubclassOf<AUnit> SpawnedClass_;

	UPROPERTY( EditDefaultsOnly, Category = "Settings", meta = ( ClampMin = 0 ) )
	int SpawnedCount_ = 2;

	UPROPERTY( EditDefaultsOnly, Category = "Settings", meta = ( ClampMin = 0, Units = "s" ) )
	float GroupSpawnInterval_ = 5.0f;

	UPROPERTY( EditDefaultsOnly, Category = "Settings", meta = ( ClampMin = 0, Units = "s" ) )
	float UnitSpawnInterval_ = 0.0f;

	UPROPERTY( EditDefaultsOnly, Category = "Settings" )
	bool SpawnOnlyWhenSeesEnemy_ = false;

	UPROPERTY( EditDefaultsOnly, Category = "Settings" )
	bool StopWhileSpawning_ = true;

	UPROPERTY( EditDefaultsOnly, Category = "Settings", meta = ( ClampMin = 0, Units = "s" ) )
	float StopTimeBeforeSpawn_ = 0.0f;

	UPROPERTY()
	TObjectPtr<UUnitBuilder> UnitBuilder_;

	FTimerHandle GroupSpawnTimer_;
	FTimerHandle UnitSpawnTimer_;
	FTimerHandle MovementTimer_;

	// Units left to spawn in the current group (per component, used to be a function-level static)
	int32 RemainingInGroup_ = 0;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> ResolvedSpawnAbilityVFX_;
};
