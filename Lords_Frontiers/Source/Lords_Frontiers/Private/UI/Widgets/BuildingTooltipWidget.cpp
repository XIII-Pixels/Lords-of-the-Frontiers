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

		//
		// UResourceGenerator* gen = resB->GetResourceGenerator();
		// if ( gen )
		// {
		// 	TMap<EResourceType, int32> prodMap = gen->GetTotalProduction();
		//
		// 	FResourceProduction prod;
		// 	prod.Gold = prodMap.Contains( EResourceType::Gold ) ? prodMap[EResourceType::Gold] : 0;
		// 	prod.Food = prodMap.Contains( EResourceType::Food ) ? prodMap[EResourceType::Food] : 0;
		// 	prod.Population = prodMap.Contains( EResourceType::Population ) ? prodMap[EResourceType::Population] : 0;
		// 	prod.Progress = prodMap.Contains( EResourceType::Progress ) ? prodMap[EResourceType::Progress] : 0;
		//
		// 	Text_Production->SetText(
		// 	    FText::Format( FText::FromString( TEXT( "Income | {0}" ) ), FormatResourceText( prod ) )
		// 	);
		// 	Text_Production->SetVisibility( ESlateVisibility::Visible );
	}
// }
// else
// {
// 	Text_Production->SetVisibility( ESlateVisibility::Collapsed );
// }
}

FText UBuildingTooltipWidget::FormatResourceText( const FResourceProduction& production )
{
	return FText::Format(
	    FText::FromString( TEXT( "Gold: {0}, Food: {1}, Population: {2}, Progress: {3}" ) ), production.Gold,
	    production.Food, production.Population, production.Progress
	);
}
