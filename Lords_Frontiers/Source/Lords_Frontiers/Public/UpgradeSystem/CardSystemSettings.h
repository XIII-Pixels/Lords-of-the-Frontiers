#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CardSystemSettings.generated.h"

class ACardVisualizer;

UCLASS()
class LORDS_FRONTIERS_API UCardSystemSettings : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Cards")
    TSubclassOf<ACardVisualizer> CardVisualizerClass;

    // Расстояние от камеры до карт
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Cards")
    float DistanceFromCamera = 300.0f;

    // Расстояние между картами
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Cards")
    float SpacingBetweenCards = 150.0f;
};