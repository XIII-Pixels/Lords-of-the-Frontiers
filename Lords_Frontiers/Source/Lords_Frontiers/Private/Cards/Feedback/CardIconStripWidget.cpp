#include "Cards/Feedback/CardIconStripWidget.h"

#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"

int32 UCardIconStripWidget::AddIcon( UTexture2D* icon )
{
	if ( !icon || !IconBox )
	{
		return INDEX_NONE;
	}

	UImage* image = NewObject<UImage>( this );
	if ( !image )
	{
		return INDEX_NONE;
	}

	image->SetBrushFromTexture( icon );
	image->SetDesiredSizeOverride( IconSize );

	UHorizontalBoxSlot* boxSlot = IconBox->AddChildToHorizontalBox( image );
	if ( boxSlot )
	{
		boxSlot->SetPadding( SlotPadding );
		boxSlot->SetVerticalAlignment( VAlign_Center );
	}

	const int32 slotId = NextSlotId_++;
	Slots_.Add( slotId, image );
	return slotId;
}

void UCardIconStripWidget::RemoveIcon( int32 slotId )
{
	TObjectPtr<UImage>* found = Slots_.Find( slotId );
	if ( !found )
	{
		return;
	}

	if ( UImage* image = found->Get() )
	{
		image->RemoveFromParent();
	}
	Slots_.Remove( slotId );
}

void UCardIconStripWidget::ClearAll()
{
	for ( const TPair<int32, TObjectPtr<UImage>>& pair : Slots_ )
	{
		if ( UImage* image = pair.Value.Get() )
		{
			image->RemoveFromParent();
		}
	}
	Slots_.Empty();
}
