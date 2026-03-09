#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "BonusSingleEntry.generated.h"

class UImage;
class UTextBlock;
/** (Maxim)
 */
UCLASS()
class LORDS_FRONTIERS_API UBonusSingleEntry : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable )
	void SetData( UTexture2D* icon, float value, bool bIsNegative );

protected:
	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UTextBlock> ValueText;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UImage> IconImage;
};