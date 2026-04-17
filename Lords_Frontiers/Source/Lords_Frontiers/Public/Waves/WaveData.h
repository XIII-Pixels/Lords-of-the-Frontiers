#pragma once

#include "Lords_Frontiers/Public/Waves/Wave.h"

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "WaveData.generated.h"

UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UWaveData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Wave" )
	FWave Wave;
};