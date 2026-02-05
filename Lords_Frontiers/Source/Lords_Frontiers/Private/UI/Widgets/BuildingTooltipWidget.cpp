#include "UI/Widgets/BuildingTooltipWidget.h"

#include "Building/Building.h"
#include "Building/ResourceBuilding.h"

#include "Components/TextBlock.h"

void UBuildingTooltipWidget::UpdateTooltip( ABuilding* buildingCDO )
{
	if ( !buildingCDO )
	{
		return;
	}

	Text_BuildingName->SetText( FText::FromString( buildingCDO->GetNameBuild() ) );

	Text_Cost->SetText(
	    FText::Format( FText::FromString( TEXT( "Cost | {0}" ) ), FormatResourceText( buildingCDO->GetBuildingCost() ) )
	);

	Text_Maintenance->SetText( FText::Format(
	    FText::FromString( TEXT( "Rent | {0}" ) ), FormatResourceText( buildingCDO->GetMaintenanceCost() )
	) );

	if ( AResourceBuilding* resB = Cast<AResourceBuilding>( buildingCDO ) )
	{
		Text_Production->SetText( FText::Format(
			FText::FromString( TEXT( "Income | {0}" ) ), FormatResourceText( resB->GetProductionConfig() )
		) );
	}
}

FText UBuildingTooltipWidget::FormatResourceText( const FResourceProduction& production )
{
	return FText::Format(
	    FText::FromString( TEXT( "Gold: {0}, Food: {1}, Population: {2}, Progress: {3}" ) ), production.Gold,
	    production.Food, production.Population, production.Progress
	);
}
