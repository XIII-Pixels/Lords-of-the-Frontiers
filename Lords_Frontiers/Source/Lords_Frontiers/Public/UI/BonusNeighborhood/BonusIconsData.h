#pragma once

#include "Building/Bonus/BuildingBonusEntry.h"
#include "Resources/GameResource.h"

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"

#include "BonusIconsData.generated.h"

class UVerticalBox;
class UImage;
class UTextBlock;
/** (Maxim)
 */
UCLASS()
class LORDS_FRONTIERS_API UBonusIconsData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Icons|Resources" )
	TMap<EResourceType, TObjectPtr<UTexture2D>> ResourceIcons;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Icons|Stats" )
	TMap<EStatsType, TObjectPtr<UTexture2D>> StatIcons;

	UTexture2D* GetIconForBonus( const FBonusIconData& data ) const
	{
		if ( data.Category == EBonusCategory::Production || data.Category == EBonusCategory::Maintenance )
		{
			const TObjectPtr<UTexture2D>* found = ResourceIcons.Find( data.ResourceType );
			return found ? *found : nullptr;
		}
		else if ( data.Category == EBonusCategory::Stats )
		{
			const TObjectPtr<UTexture2D>* found = StatIcons.Find( data.StatType );
			return found ? *found : nullptr;
		}
		return nullptr;
	}
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Highlight" )
	TObjectPtr<UMaterialInterface> HighlightMaterial;
};