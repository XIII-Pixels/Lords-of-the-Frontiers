#include "UpgradeSystem/CardManager.h"
#include "UpgradeSystem/Card.h"
#include "UpgradeSystem/UpgradeManager.h"
#include "UpgradeSystem/CardSystemSettings.h"
#include "UpgradeSystem/CardSelectionWidget.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void UCardManager::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    LoadSettings();
    LoadCardsFromFolder();
}

void UCardManager::LoadSettings()
{
    Settings = LoadObject<UCardSystemSettings>(nullptr, TEXT("/Game/Cards/DA_CardSettings.DA_CardSettings"));

    if (!Settings)
    {
        UE_LOG(LogTemp, Error, TEXT("CardManager: Failed to load CardSystemSettings!"));
    }
}

void UCardManager::LoadCardsFromFolder()
{
    FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

    TArray<FAssetData> AssetList;
    AssetRegistry.Get().GetAssetsByPath(TEXT("/Game/Cards/CardData"), AssetList, true);

    for (const FAssetData& Asset : AssetList)
    {
        if (UCard* Card = Cast<UCard>(Asset.GetAsset()))
        {
            AvailableCards.Add(Card);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("CardManager: Loaded %d cards"), AvailableCards.Num());
}

TArray<UCard*> UCardManager::GetRandomCards(int32 Count)
{
    TArray<UCard*> Result;

    if (AvailableCards.Num() == 0)
    {
        return Result;
    }

    TArray<UCard*> TempCards = AvailableCards;

    for (int32 i = 0; i < Count && TempCards.Num() > 0; i++)
    {
        int32 RandomIndex = FMath::RandRange(0, TempCards.Num() - 1);
        Result.Add(TempCards[RandomIndex]);
        TempCards.RemoveAt(RandomIndex);
    }

    return Result;
}

void UCardManager::ShowCardSelection(int32 Count)
{
    UE_LOG(LogTemp, Warning, TEXT("ShowCardSelection: Start"));

    HideCardSelection();

    if (!Settings || !Settings->CardSelectionWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("ShowCardSelection: No settings or widget class!"));
        return;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("ShowCardSelection: No PlayerController!"));
        return;
    }

    CurrentSelectionWidget = CreateWidget<UCardSelectionWidget>(PC, Settings->CardSelectionWidgetClass);
    if (!CurrentSelectionWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("ShowCardSelection: Failed to create widget!"));
        return;
    }

    TArray<UCard*> Cards = GetRandomCards(Count);
    UE_LOG(LogTemp, Warning, TEXT("ShowCardSelection: Got %d cards"), Cards.Num());

    CurrentSelectionWidget->AddToViewport();
    CurrentSelectionWidget->ShowCards(Cards);
}

void UCardManager::HideCardSelection()
{
    if (CurrentSelectionWidget)
    {
        CurrentSelectionWidget->RemoveFromParent();
        CurrentSelectionWidget = nullptr;
    }
}

void UCardManager::SelectCard(UCard* Card)
{
    if (UUpgradeManager* UpgradeManager = GetWorld()->GetSubsystem<UUpgradeManager>())
    {
        UpgradeManager->AddCard(Card);
    }

    HideCardSelection(); 
}