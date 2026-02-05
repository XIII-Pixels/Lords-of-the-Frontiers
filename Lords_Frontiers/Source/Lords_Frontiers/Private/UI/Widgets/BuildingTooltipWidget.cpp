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

	Text_Cost->SetText( FText::Format(
	    FText::FromString( TEXT( "Стоимость | {0}" ) ), FormatResourceText( buildingCDO->GetBuildingCost() )
	) );

	Text_Maintenance->SetText( FText::Format(
	    FText::FromString( TEXT( "Содержание | {0}" ) ), FormatResourceText( buildingCDO->GetMaintenanceCost() )
	) );

	if ( AResourceBuilding* resB = Cast<AResourceBuilding>( buildingCDO ) )
	{
		Text_Production->SetText( FText::Format(
		    FText::FromString( TEXT( "Доход | {0}" ) ), FormatResourceText( resB->GetProductionConfig() )
		) );
	}
	else
	{
		Text_Production->SetText( FText::FromString( "" ) );
	}
}

FText UBuildingTooltipWidget::FormatResourceText( const FResourceProduction& production )
{
	return FText::Format(
	    FText::FromString( TEXT( "Золото: {0}, Еда: {1}, Население: {2}, Прогресс: {3}" ) ), production.Gold,
	    production.Food, production.Population, production.Progress
	);
}
