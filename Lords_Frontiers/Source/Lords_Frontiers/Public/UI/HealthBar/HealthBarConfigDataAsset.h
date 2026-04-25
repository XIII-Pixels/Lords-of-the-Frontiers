#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "HealthBarConfigDataAsset.generated.h"

class UHealthBarWidget;
class AUnit;
class ABuilding;

UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UHealthBarConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|HealthBar|Default" )
	TSubclassOf<UHealthBarWidget> DefaultBarClass;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|HealthBar|Overrides" )
	TMap<TSubclassOf<AUnit>, TSubclassOf<UHealthBarWidget>> EnemyOverrides;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|HealthBar|Overrides" )
	TMap<TSubclassOf<ABuilding>, TSubclassOf<UHealthBarWidget>> BuildingOverrides;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|HealthBar|Behavior", meta = ( Units = "s" ) )
	float HideDelay_ = 3.0f;

	UPROPERTY(
	    EditAnywhere, BlueprintReadOnly, Category = "Settings|HealthBar|Pool", meta = ( ClampMin = "0" )
	)
	int32 PoolSize_ = 64;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|HealthBar|Placement" )
	float WorldOffsetZ_ = 120.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|HealthBar|Placement" )
	FVector2D WorldDrawSize_ = FVector2D( 150.0f, 20.0f );
};
