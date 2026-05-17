#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"

#include "AudioSettingsSave.generated.h"

UCLASS()
class LORDS_FRONTIERS_API UAudioSettingsSave : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY( VisibleAnywhere, Category = "Audio" )
	float MusicVolume = 0.3f;

	UPROPERTY( VisibleAnywhere, Category = "Audio" )
	float AmbientVolume = 0.3f;

	UPROPERTY( VisibleAnywhere, Category = "Audio" )
	float EffectsVolume = 0.3f;

	UPROPERTY( VisibleAnywhere, Category = "Audio" )
	float UIVolume = 0.3f;
};
