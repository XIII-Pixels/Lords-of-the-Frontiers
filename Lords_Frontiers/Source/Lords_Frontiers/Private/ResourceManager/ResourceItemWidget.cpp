#include "Lords_Frontiers/Public/ResourceManager/ResourceItemWidget.h"

void UResourceItemWidget::SetupItem(UTexture2D* IconTexture, int32 InitialValue, int32 MaxValue)
{
	if (ResourceIcon && IconTexture)
	{
		ResourceIcon->SetBrushFromTexture(IconTexture);
	}
	UpdateAmount(InitialValue, MaxValue);
}

void UResourceItemWidget::UpdateAmount(int32 NewAmount, int32 MaxAmount)
{
    if (ResourceAmountText)
    {
        FString ResourceString = FString::Printf(TEXT("%d / %d"), FMath::Max(0, NewAmount), MaxAmount);
        ResourceAmountText->SetText(FText::FromString(ResourceString));
    }
}