#include "UpgradeSystem/CardSelectionWidget.h"
#include "UpgradeSystem/CardWidget.h"
#include "UpgradeSystem/CardManager.h"
#include "UpgradeSystem/Card.h"
#include "Components/HorizontalBox.h"

void UCardSelectionWidget::ShowCards(const TArray<UCard*>& Cards)
{
    if (!CardsContainer || !CardWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("CardSelectionWidget: Missing container or widget class!"));
        return;
    }

    CardsContainer->ClearChildren();
    SpawnedCardWidgets.Empty();

    for (UCard* Card : Cards)
    {
        UCardWidget* CardWidget = CreateWidget<UCardWidget>(this, CardWidgetClass);
        if (CardWidget)
        {
            CardWidget->SetCard(Card);
            CardWidget->OnCardSelected.AddDynamic(this, &UCardSelectionWidget::OnCardSelected);
            CardsContainer->AddChild(CardWidget);
            SpawnedCardWidgets.Add(CardWidget);
        }
    }
}

void UCardSelectionWidget::Hide()
{
    RemoveFromParent();
}

void UCardSelectionWidget::OnCardSelected(UCard* SelectedCard)
{
    if (UCardManager* CardManager = GetWorld()->GetSubsystem<UCardManager>())
    {
        CardManager->SelectCard(SelectedCard);
    }

    Hide();
}