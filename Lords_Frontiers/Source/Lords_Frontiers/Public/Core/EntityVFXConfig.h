#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "EntityVFXConfig.generated.h"

class UNiagaraSystem;
class AUnit;
class ABuilding;

USTRUCT( BlueprintType )
struct FUnitVFXOverride
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	TObjectPtr<UNiagaraSystem> DeathVFX = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	TObjectPtr<UNiagaraSystem> HitVFX = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( ClampMin = "0.0", Units = "s" ) )
	float DeathDestroyDelay = -1.0f;
};

USTRUCT( BlueprintType )
struct FBuildingVFXOverride
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	TObjectPtr<UNiagaraSystem> HitVFX = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	TObjectPtr<UNiagaraSystem> DestructionVFX = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	TObjectPtr<UNiagaraSystem> ConstructionVFX = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( ClampMin = "-1.0", Units = "s" ) )
	float RuinDelay = -1.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( ClampMin = "-1.0", Units = "s" ) )
	float ConstructionDelay = -1.0f;
};

/**
 * m
 */
UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UEntityVFXConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Unit|Defaults" )
	TObjectPtr<UNiagaraSystem> DefaultUnitDeathVFX = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Unit|Defaults" )
	TObjectPtr<UNiagaraSystem> DefaultUnitHitVFX = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Unit|Defaults", meta = ( ClampMin = "0.0", Units = "s" ) )
	float DefaultDeathDestroyDelay = 1.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Unit|Overrides" )
	TMap<TSubclassOf<AUnit>, FUnitVFXOverride> UnitOverrides;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Building|Defaults" )
	TObjectPtr<UNiagaraSystem> DefaultBuildingHitVFX = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Building|Defaults" )
	TObjectPtr<UNiagaraSystem> DefaultBuildingDestructionVFX = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Building|Defaults" )
	TObjectPtr<UNiagaraSystem> DefaultBuildingConstructionVFX = nullptr;

	UPROPERTY(
	    EditAnywhere, BlueprintReadOnly, Category = "Building|Defaults", meta = ( ClampMin = "0.0", Units = "s" )
	)
	float DefaultRuinDelay = 0.0f;

	UPROPERTY(
	    EditAnywhere, BlueprintReadOnly, Category = "Building|Defaults", meta = ( ClampMin = "0.0", Units = "s" )
	)
	float DefaultConstructionDelay = 0.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Building|Overrides" )
	TMap<TSubclassOf<ABuilding>, FBuildingVFXOverride> BuildingOverrides;
};