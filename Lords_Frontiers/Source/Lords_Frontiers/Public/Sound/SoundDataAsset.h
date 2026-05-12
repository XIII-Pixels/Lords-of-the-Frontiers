#pragma once

#include "GameplayTagContainer.h"

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Sound/SoundBase.h"

#include "SoundDataAsset.generated.h"

USTRUCT( BlueprintType )
struct FSoundEntry
{
	GENERATED_BODY()

	UPROPERTY( EditDefaultsOnly )
	TObjectPtr<USoundBase> Sound = nullptr;

	UPROPERTY( EditDefaultsOnly )
	TObjectPtr<USoundAttenuation> Attenuation = nullptr;

	UPROPERTY( EditDefaultsOnly )
	bool bIs3D = false;

	UPROPERTY(
	    EditDefaultsOnly, BlueprintReadOnly, Category = "Variance",
	    meta = ( UIMin = "0.1", UIMax = "4.0", ClampMin = "0.1", ClampMax = "4.0" )
	)
	FVector2D PitchRange = FVector2D( 1.f, 1.f );

	UPROPERTY(
	    EditDefaultsOnly, BlueprintReadOnly, Category = "Variance",
	    meta = ( UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0" )
	)
	FVector2D VolumeRange = FVector2D( 1.f, 1.f );
};

UCLASS( BlueprintType )
class LORDS_FRONTIERS_API USoundDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	const FSoundEntry* FindByTag( const FGameplayTag& tag );

private:
	UPROPERTY( EditDefaultsOnly, Category = "Settings" )
	TMap<FGameplayTag, FSoundEntry> Entries_;
};
