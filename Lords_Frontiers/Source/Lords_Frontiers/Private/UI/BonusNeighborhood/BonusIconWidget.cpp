#include "UI/BonusNeighborhood/BonusIconWidget.h"

#include "UI/BonusNeighborhood/BonusSingleEntry.h"

#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void UBonusIconWidget::SetBuildingIcon( UTexture2D* icon )
{
	if ( !BuildingIcon_ )
	{
		return;
	}

	if ( icon )
	{
		BuildingIcon_->SetBrushFromTexture( icon );
		BuildingIcon_->SetVisibility( ESlateVisibility::HitTestInvisible );
	}
	else
	{
		BuildingIcon_->SetVisibility( ESlateVisibility::Collapsed );
	}
}

void UBonusIconWidget::AddEntry( const FBonusIconData& data )
{
	if ( !BonusContainer_ || !EntryWidgetClass )
	{
		return;
	}

	UBonusSingleEntry* entry = CreateWidget<UBonusSingleEntry>( GetOwningPlayer(), EntryWidgetClass );

	if ( !entry )
	{
		return;
	}

	UTexture2D* icon = IconsData ? IconsData->GetIconForBonus( data ) : data.Icon.Get();

	bool bIsNegative = ( data.Value < 0.0f );
	entry->SetData( icon, data.Value, bIsNegative );

	UVerticalBoxSlot* slot = BonusContainer_->AddChildToVerticalBox( entry );

	if ( slot )
	{
		slot->SetPadding( FMargin( 0.0f, 1.0f ) );
	}
}