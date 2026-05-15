#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Tutorial/TutorialTypes.h"

#include "TutorialConfig.generated.h"

class UMaterialInterface;

UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UTutorialConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Tutorial" )
	FName TutorialMapName;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Tutorial" )
	TSoftObjectPtr<UMaterialInterface> HighlightOverlayMaterial;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Tutorial", meta = ( TitleProperty = "WidgetClass" ) )
	TArray<FTutorialStep> Steps;
};
