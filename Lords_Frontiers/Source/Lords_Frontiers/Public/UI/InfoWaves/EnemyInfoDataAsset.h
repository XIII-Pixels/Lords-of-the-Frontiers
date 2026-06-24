#pragma once


#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "EnemyInfoDataAsset.generated.h"

class AUnit;

USTRUCT( BlueprintType )
struct FEnemyUIData
{
	GENERATED_BODY()

	/**
	 * Id of this enemy's texts in ST_GameStrings: Enemy.Name.<Id> / Enemy.Description.<Id>
	 * when those keys exist. None = the unit class name without the "BP_" prefix and "_C"
	 * suffix. EnemyName/EnemyDescription below are the fallback for missing keys.
	 */
	UPROPERTY( EditAnywhere, Category = "Settings|UI" )
	FName LocalizationId = NAME_None;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|UI",
		meta = ( ToolTip = "Fallback name, shown only when ST_GameStrings has no Enemy.Name.<Id> entry." ) )
	FText EnemyName;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|UI",
		meta = ( ToolTip = "Fallback description, shown only when ST_GameStrings has no Enemy.Description.<Id> entry." ) )
	FText EnemyDescription;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|UI" )
	TObjectPtr<UTexture2D> EnemyIcon;
};

UCLASS( BlueprintType )
class LORDS_FRONTIERS_API UEnemyInfoDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Enemy Info" )
	TMap<TSubclassOf<AUnit>, FEnemyUIData> EnemyDataMap;

	/** Enemy name for display: ST_GameStrings entry Enemy.Name.<Id> when present, otherwise the inline EnemyName. */
	UFUNCTION( BlueprintPure, Category = "Enemy Info" )
	FText GetEnemyName( TSubclassOf<AUnit> enemyClass ) const;

	/** Enemy description for display: Enemy.Description.<Id> when present, otherwise the inline EnemyDescription. */
	UFUNCTION( BlueprintPure, Category = "Enemy Info" )
	FText GetEnemyDescription( TSubclassOf<AUnit> enemyClass ) const;
};