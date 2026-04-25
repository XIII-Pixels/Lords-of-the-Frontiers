#pragma once
#include "CoreMinimal.h"
#include "Waves/Wave.h"
#include "Lords_Frontiers/Public/Waves/WaveData.h"

#include "Engine/DataAsset.h"

#include "WaveConfig.generated.h"

UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UWaveConfigData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "WaveConfig" )
	TArray<TObjectPtr<UWaveData>> Waves;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "WaveConfig" )
	float WaveEndSafetyMargin = 1.0f;
};