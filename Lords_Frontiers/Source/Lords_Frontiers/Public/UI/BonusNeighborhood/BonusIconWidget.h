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

	void ApplyCameraScale( float zoomAlpha );

	float MinScale() const
	{
		return MinScale_;
	}

	float MaxScale() const
	{
		return MaxScale_;
	}

	UPROPERTY( EditAnywhere, Category = "Settings|Bonus" )
	TObjectPtr<UBonusIconsData> IconsData;

	UPROPERTY( EditAnywhere, Category = "Settings|Bonus" )
	TSubclassOf<UBonusSingleEntry> EntryWidgetClass;

protected:
	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UVerticalBox> BonusContainer_;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UImage> BuildingIcon_;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Bonus|Scale" )
	float MinScale_ = 0.5f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Bonus|Scale" )
	float MaxScale_ = 1.5f;
};