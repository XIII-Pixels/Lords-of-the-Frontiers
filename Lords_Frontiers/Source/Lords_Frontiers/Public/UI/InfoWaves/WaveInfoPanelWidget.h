#pragma once

#include "Units/Unit.h"

#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h" 
#include "CoreMinimal.h"

#include "WaveInfoPanelWidget.generated.h"

class UVerticalBox;
class UEnemyInfoDataAsset;
class UEnemyRowWidget;

UCLASS( Abstract )
class LORDS_FRONTIERS_API UWaveInfoPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void PopulatePanel( const TMap<TSubclassOf<AUnit>, int32>& WaveData );

	UFUNCTION( BlueprintImplementableEvent, Category = "UI|Animation" )
	void PlaySlideInAnimation();

	UFUNCTION( BlueprintImplementableEvent, Category = "UI|Animation" )
	void PlaySlideOutAnimation();

protected:
	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UHorizontalBox> EnemyListContainer;

	UPROPERTY( EditAnywhere, Category = "Settings|Data" )
	TObjectPtr<UEnemyInfoDataAsset> EnemyDataAsset;

	UPROPERTY( EditAnywhere, Category = "Settings|Classes" )
	TSubclassOf<UEnemyRowWidget> EnemyRowClass;
};