#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Resources/GameResource.h" 
#include "EntityStats.h"

#include "BuildingUIConfig.generated.h"

class ABuilding;

USTRUCT( BlueprintType )
struct FBuildingUIData
{
	GENERATED_BODY()

	/**
	 * Id of this building's texts in ST_GameStrings: the tooltip shows Building.Name.<Id> and
	 * Building.Description.<Id> when those keys exist. None = the building class name without
	 * the "BP_" prefix and "_C" suffix. Name/Description below are the fallback for missing keys.
	 */
	UPROPERTY( EditAnywhere )
	FName LocalizationId = NAME_None;

	UPROPERTY( EditAnywhere,
		meta = ( ToolTip = "Fallback name, shown only when ST_GameStrings has no Building.Name.<Id> entry." ) )
	FText Name;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( MultiLine = true,
		ToolTip = "Fallback description, shown only when ST_GameStrings has no Building.Description.<Id> entry." ) )
	FText Description;

	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	TObjectPtr<UTexture2D> Icon;
};

UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UBuildingUIConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Config" )
	TMap<TSubclassOf<ABuilding>, FBuildingUIData> BuildingsData;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Config|Global Icons" )
	TMap<EResourceType, TObjectPtr<UTexture2D>> ResourceIcons;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Config|Global Icons" )
	TMap<EStatsType, TObjectPtr<UTexture2D>> StatIcons;

	/** Building name for display: ST_GameStrings entry Building.Name.<Id> when present, otherwise the inline Name. */
	UFUNCTION( BlueprintPure, Category = "Config" )
	FText GetBuildingName( TSubclassOf<ABuilding> buildingClass ) const;

	/** Building description for display: Building.Description.<Id> when present, otherwise the inline Description. */
	UFUNCTION( BlueprintPure, Category = "Config" )
	FText GetBuildingDescription( TSubclassOf<ABuilding> buildingClass ) const;
};