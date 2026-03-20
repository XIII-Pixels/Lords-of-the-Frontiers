#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "BuildingUIConfig.generated.h"

class ABuilding;

USTRUCT( BlueprintType )
struct FBuildingUIData
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	FText Name;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( MultiLine = true ) )
	FText Description;

	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	TObjectPtr<UTexture2D> Icon;
};

UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UBuildingUIConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Config" )
	TMap<TSubclassOf<ABuilding>, FBuildingUIData> BuildingsData;
};