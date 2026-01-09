#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CardSelectionWidget.generated.h"

class UCard;
class UCardWidget;
class UHorizontalBox;

UCLASS()
class LORDS_FRONTIERS_API UCardSelectionWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void ShowCards(const TArray<UCard*>& Cards);
    void Hide();

protected:
    UFUNCTION()
    void OnCardSelected(UCard* SelectedCard);

    UPROPERTY(meta = (BindWidget))
    UHorizontalBox* CardsContainer;

    UPROPERTY(EditDefaultsOnly, Category = "Setting | Cards")
    TSubclassOf<UCardWidget> CardWidgetClass;

    UPROPERTY()
    TArray<UCardWidget*> SpawnedCardWidgets;
};