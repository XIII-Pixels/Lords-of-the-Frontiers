#include "UpgradeSystem/CardWidget.h"
#include "UpgradeSystem/Card.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UCardWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (CardButton)
    {
        CardButton->OnClicked.AddDynamic(this, &UCardWidget::OnButtonClicked);
    }
}

void UCardWidget::SetCard(UCard* Card)
{
    CurrentCard = Card;

    if (!Card)
    {
        return;
    }

    if (CardImage && Card->CardTexture)
    {
        CardImage->SetBrushFromTexture(Card->CardTexture);
    }
}

void UCardWidget::OnButtonClicked()
{
    if (CurrentCard)
    {
        OnCardSelected.Broadcast(CurrentCard);
    }
}