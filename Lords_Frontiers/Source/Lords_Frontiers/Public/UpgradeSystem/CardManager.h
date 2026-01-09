#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CardManager.generated.h"

class UCard;
class UCardSystemSettings;
class UCardSelectionWidget;

UCLASS()
class LORDS_FRONTIERS_API UCardManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    TArray<UCard*> GetRandomCards(int32 Count);
    void SelectCard(UCard* Card);
    void ShowCardSelection(int32 Count = 3);
    void HideCardSelection();

private:
    void LoadCardsFromFolder();
    void LoadSettings();

    UPROPERTY()
    TArray<UCard*> AvailableCards;

    UPROPERTY()
    UCardSystemSettings* Settings;

    UPROPERTY()
    UCardSelectionWidget* CurrentSelectionWidget;
};