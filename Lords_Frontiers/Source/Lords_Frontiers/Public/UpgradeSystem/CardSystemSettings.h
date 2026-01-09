#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CardSystemSettings.generated.h"

class UCardSelectionWidget;

UCLASS()
class LORDS_FRONTIERS_API UCardSystemSettings : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting | Cards")
    TSubclassOf<UCardSelectionWidget> CardSelectionWidgetClass;
};