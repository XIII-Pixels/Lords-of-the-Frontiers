#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CardWidget.generated.h"

class UCard;
class UImage;
class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCardSelected, UCard*, SelectedCard);

UCLASS()
class LORDS_FRONTIERS_API UCardWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetCard(UCard* Card);

    UPROPERTY(BlueprintAssignable)
    FOnCardSelected OnCardSelected;

protected:
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnButtonClicked();

    UPROPERTY(meta = (BindWidget))
    UImage* CardImage;

    UPROPERTY(meta = (BindWidget))
    UButton* CardButton;

    UPROPERTY()
    UCard* CurrentCard;
};