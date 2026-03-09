#include "UI/BonusNeighborhood/BonusSingleEntry.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UBonusSingleEntry::SetData( UTexture2D* icon, float value, bool bIsNegative )
{
	if ( ValueText )
	{
		const int32 roundedValue = FMath::RoundToInt( FMath::Abs( value ) );

		FString text = bIsNegative ? FString::Printf( TEXT( "-%d" ), roundedValue )
		                           : FString::Printf( TEXT( "+%d" ), roundedValue );

		ValueText->SetText( FText::FromString( text ) );

		FSlateColor color = bIsNegative ? FSlateColor( FLinearColor::Red ) : FSlateColor( FLinearColor::Green );
		ValueText->SetColorAndOpacity( color );
	}

	if ( IconImage && icon )
	{
		IconImage->SetBrushFromTexture( icon );
	}
}