#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "WaveInfoPanelWidget.generated.h"

class AUnit;
class UHorizontalBox;
class UEnemyInfoDataAsset;
class UEnemyRowWidget;

UCLASS( Abstract )
class LORDS_FRONTIERS_API UWaveInfoPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void PopulatePanel( const TMap<TSubclassOf<AUnit>, int32>& waveData );

	UFUNCTION( BlueprintImplementableEvent, Category = "UI|Animation|Settings" )
	void PlaySlideInAnimation();

	UFUNCTION( BlueprintImplementableEvent, Category = "UI|Animation|Settings" )
	void PlaySlideOutAnimation();

protected:
	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UHorizontalBox> EnemyListContainer;

	UPROPERTY( EditAnywhere, Category = "Settings|Data" )
	TObjectPtr<UEnemyInfoDataAsset> EnemyDataAsset;

	UPROPERTY( EditAnywhere, Category = "Settings|Classes" )
	TSubclassOf<UEnemyRowWidget> EnemyRowClass;
};