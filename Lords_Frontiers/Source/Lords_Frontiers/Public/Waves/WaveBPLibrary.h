
#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Wave.h"
#include "WaveBPLibrary.generated.h"


 // Blueprint-callable wrappers for functions operating on FWave (because UFUNCTION can't be in USTRUCT)
 //(Artyom)
UCLASS()
class LORDS_FRONTIERS_API UWaveBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Blueprint wrapper to get spawn transform for a group (calls FWave::GetSpawnTransformForGroup)
	UFUNCTION ( BlueprintCallable, Category = "Wave", meta = ( WorldContext = "WorldContextObject" ) )
	static FTransform GetSpawnTransformForGroup_BP ( const FWave& wave, UObject* worldContextObject, int32 groupIndex )
	{
		return wave.GetSpawnTransformForGroup ( worldContextObject, groupIndex );
	}

	//Blueprint wrapper: get total wave duration 
	UFUNCTION ( BlueprintPure, Category = "Wave" )
	static float GetTotalWaveDuration_BP ( const FWave& wave )
	{
		return wave.GetTotalWaveDuration ();
	}

	// Blueprint wrapper: is wave valid 
	UFUNCTION ( BlueprintPure, Category = "Wave" )
	static bool IsWaveValid_BP ( const FWave& wave )
	{
		return wave.IsValid ();
	}
};
//*/