#pragma once

#include "BonusIconsData.h"
#include "Building/Bonus/BuildingBonusEntry.h"

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "BonusIconWidget.generated.h"

class UVerticalBox;
class UImage;
class UTextBlock;
class UBonusSingleEntry;
/** (Maxim)
 */
UCLASS()
class LORDS_FRONTIERS_API UBonusIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, Category = "Settings|Bonus" )
	void SetBuildingIcon( UTexture2D* icon );

	UFUNCTION( BlueprintCallable, Category = "Settings|Bonus" )
	void AddEntry( const FBonusIconData& data );

	UPROPERTY( EditAnywhere, Category = "Settings|Bonus" )
	TObjectPtr<UBonusIconsData> IconsData;

	UPROPERTY( EditAnywhere, Category = "Settings|Bonus" )
	TSubclassOf<UBonusSingleEntry> EntryWidgetClass;

protected:
	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UVerticalBox> BonusContainer_;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UImage> BuildingIcon_;
};