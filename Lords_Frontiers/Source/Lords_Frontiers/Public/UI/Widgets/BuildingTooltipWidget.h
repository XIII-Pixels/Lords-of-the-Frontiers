#pragma once

#include "Lords_Frontiers/Public/Resources/GameResource.h"

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "BuildingTooltipWidget.generated.h"

class UTextBlock;

UCLASS( Abstract )
class LORDS_FRONTIERS_API UBuildingTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateTooltip( class ABuilding* buildingCDO );

protected:
	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UTextBlock> Text_BuildingName;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UTextBlock> Text_Cost;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UTextBlock> Text_Maintenance;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UTextBlock> Text_Production;

private:

	FText FormatResourceText( const FResourceProduction& production );
};