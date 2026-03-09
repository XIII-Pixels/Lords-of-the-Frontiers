#pragma once


#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "EnemyInfoDataAsset.generated.h"

class AUnit;

USTRUCT( BlueprintType )
struct FEnemyUIData
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|UI" )
	FText EnemyName;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|UI" )
	FText EnemyDescription;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|UI" )
	TObjectPtr<UTexture2D> EnemyIcon;
};

UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UEnemyInfoDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Enemy Info" )
	TMap<TSubclassOf<AUnit>, FEnemyUIData> EnemyDataMap;
};