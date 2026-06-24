#pragma once

#include "UI/Widgets/BuildingTooltipWidget.h"

#include "CoreMinimal.h"

#include "SelectedBuildingPanelWidget.generated.h"

class ABuilding;
class UBuildingActionButtonWidget;

UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API USelectedBuildingPanelWidget : public UBuildingTooltipWidget
{
	GENERATED_BODY()

public:
	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UBuildingActionButtonWidget> ButtonRelocate;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UBuildingActionButtonWidget> ButtonRemove;

	void ShowForBuilding( ABuilding* building );
	void HidePanel( bool bAnimate = true );

protected:
	virtual void NativeConstruct() override;

	virtual ESlateVisibility GetShownVisibility() const override
	{
		return ESlateVisibility::SelfHitTestInvisible;
	}
};
