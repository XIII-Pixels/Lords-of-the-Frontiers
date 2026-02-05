#include "UI/Widgets/BuildingTooltipWidget.h"

#include "Building/Building.h"
#include "Building/ResourceBuilding.h"

#include "Components/TextBlock.h"

void UBuildingTooltipWidget::UpdateTooltip( ABuilding* buildingCDO )
{
	if ( !buildingCDO )
		return;

	Text_BuildingName->SetText( FText::FromString( buildingCDO->GetNameBuild() ) );

	FText CostVal = FormatResourceText( buildingCDO->GetBuildingCost() );
	Text_Cost->SetText( FText::Format( FText::FromString( TEXT( "Cost: {0}" ) ), CostVal ) );

	FText MaintVal = FormatResourceText( buildingCDO->GetMaintenanceCost() );
	Text_Maintenance->SetText( FText::Format( FText::FromString( TEXT( "Rent: {0}" ) ), MaintVal ) );

	if ( AResourceBuilding* resB = Cast<AResourceBuilding>( buildingCDO ) )
	{
		UResourceGenerator* gen = resB->GetResourceGenerator();
		if ( gen )
		{
			TMap<EResourceType, int32> prodMap = gen->GetTotalProduction();

			bool bHasProduction = false;
			for ( auto& pair : prodMap )
			{
				if ( pair.Value > 0 )
					bHasProduction = true;
			}

			if ( bHasProduction )
			{
				TArray<FString> parts;
				if ( prodMap.Contains( EResourceType::Gold ) && prodMap[EResourceType::Gold] > 0 )
					parts.Add( FString::Printf( TEXT( "Gold +%d" ), prodMap[EResourceType::Gold] ) );
				if ( prodMap.Contains( EResourceType::Food ) && prodMap[EResourceType::Food] > 0 )
					parts.Add( FString::Printf( TEXT( "Food +%d" ), prodMap[EResourceType::Food] ) );

				Text_Production->SetText( FText::FromString( FString::Join( parts, TEXT( ", " ) ) ) );
				Text_Production->SetVisibility( ESlateVisibility::Visible );
			}
			else
			{
				Text_Production->SetVisibility( ESlateVisibility::Collapsed );
			}
		}
	}
	else
	{
		Text_Production->SetVisibility( ESlateVisibility::Collapsed );
	}
}

FText UBuildingTooltipWidget::FormatResourceText( const FResourceProduction& production )
{
	TArray<FString> lines;
	if ( production.Gold > 0 )
		lines.Add( FString::Printf( TEXT( "Gold: %d" ), production.Gold ) );
	if ( production.Food > 0 )
		lines.Add( FString::Printf( TEXT( "Food: %d" ), production.Food ) );
	if ( production.Population > 0 )
		lines.Add( FString::Printf( TEXT( "Population: %d" ), production.Population ) );

	if ( lines.Num() == 0 )
		return FText::FromString( TEXT( "Free" ) );
	return FText::FromString( FString::Join( lines, TEXT( ", " ) ) );
}