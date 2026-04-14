#pragma once
#include "CoreMinimal.h"
#include "Waves/Wave.h"

#include "Engine/DataAsset.h"

#include "WaveConfig.generated.h"

UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UWaveConfigData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "WaveConfig" )
	TArray<FWave> Waves;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "WaveConfig" )
	float WaveEndSafetyMargin = 1.0f;
};