#include "Lords_Frontiers/Public/UI/Widgets/SelectedBuildingPanelWidget.h"

#include "Building/Building.h"
#include "UI/Widgets/BuildingActionButtonWidget.h"

void USelectedBuildingPanelWidget::NativeConstruct()
{
	bShowBuildCost_ = false;

	Super::NativeConstruct();

	ForceHide();
}

void USelectedBuildingPanelWidget::ShowForBuilding( ABuilding* building )
{
	if ( !IsValid( building ) )
	{
		HidePanel();
		return;
	}

	ShowTooltipForBuildingInstance( building );

	if ( IsValid( ButtonRelocate ) )
	{
		ButtonRelocate->RefreshFor( building );
	}
	if ( IsValid( ButtonRemove ) )
	{
		ButtonRemove->RefreshFor( building );
	}
}

void USelectedBuildingPanelWidget::HidePanel( bool bAnimate )
{
	if ( bAnimate )
	{
		HideTooltip();
	}
	else
	{
		ForceHide();
	}
}
