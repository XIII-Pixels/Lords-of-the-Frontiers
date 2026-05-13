#pragma once

#include "Cards/Visuals/CardVisualTypes.h"

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "CardVFXAsset.generated.h"

UCLASS( BlueprintType, meta = ( DisplayName = "Card VFX" ) )
class LORDS_FRONTIERS_API UCardVFXAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "VFX",
		meta = ( DisplayName = "Конфиг визуала",
			ShowOnlyInnerProperties ) )
	FCardVisualConfig Config;
};
