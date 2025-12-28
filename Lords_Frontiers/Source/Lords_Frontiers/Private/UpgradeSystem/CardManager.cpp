// Fill out your copyright notice in the Description page of Project Settings.


#include "UpgradeSystem/CardManager.h"
#include "UpgradeSystem/Card.h"
#include "UpgradeSystem/UpgradeManager.h"
#include "UpgradeSystem/CardVisualizer.h"
#include "UpgradeSystem/CardSystemSettings.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"


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

    TArray<FAssetData> assetList;
    AssetRegistry.Get().GetAssetsByPath(TEXT("/Game/Cards"), assetList, true);

    for (const FAssetData& asset : assetList)
    {
        if (UCard* card = Cast<UCard>(asset.GetAsset()))
        {
            AvailableCards.Add(card);
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("CardManager: Loaded %d cards"), AvailableCards.Num());

}
TArray<UCard*> UCardManager::GetRandomCards(int32 count)
{
    TArray<UCard*> result;

    if (AvailableCards.Num() == 0)
    {
        return result;
    }

    TArray<UCard*> tempCards = AvailableCards;

    for (int32 i = 0; i < count && tempCards.Num() > 0; i++)
    {
        int32 randomIndex = FMath::RandRange(0, tempCards.Num() - 1);
        result.Add(tempCards[randomIndex]);
        tempCards.RemoveAt(randomIndex);
    }
    return result;
}


void UCardManager::ShowCardSelection(int32 Count)
{
    UE_LOG(LogTemp, Warning, TEXT("ShowCardSelection: Start"));

    HideCardSelection();

    if (!Settings)
    {
        UE_LOG(LogTemp, Error, TEXT("ShowCardSelection: Settings is null!"));
        return;
    }

    if (!Settings->CardVisualizerClass)
    {
        UE_LOG(LogTemp, Error, TEXT("ShowCardSelection: CardVisualizerClass is null!"));
        return;
    }

    TArray<UCard*> Cards = GetRandomCards(Count);
    UE_LOG(LogTemp, Warning, TEXT("ShowCardSelection: Got %d cards"), Cards.Num());

    APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
    if (!CameraManager)
    {
        UE_LOG(LogTemp, Error, TEXT("ShowCardSelection: CameraManager is null!"));
        return;
    }

    FVector CameraLocation = CameraManager->GetCameraLocation();
    FRotator CameraRotation = CameraManager->GetCameraRotation();
    FVector ForwardVector = CameraRotation.Vector();

    UE_LOG(LogTemp, Warning, TEXT("ShowCardSelection: Camera at %s"), *CameraLocation.ToString());

    for (int32 i = 0; i < Cards.Num(); i++)
    {
        float Offset = (i - (Cards.Num() - 1) / 2.0f) * Settings->SpacingBetweenCards;
        FVector RightVector = FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::Y);
        FVector SpawnLocation = CameraLocation + ForwardVector * Settings->DistanceFromCamera + RightVector * Offset;

        FRotator SpawnRotation = CameraRotation;

        ACardVisualizer* Visualizer = GetWorld()->SpawnActor<ACardVisualizer>(
            Settings->CardVisualizerClass,
            SpawnLocation,
            SpawnRotation
        );

        if (Visualizer)
        {
            UE_LOG(LogTemp, Warning, TEXT("ShowCardSelection: Spawned card %d at %s"), i, *SpawnLocation.ToString());
            Visualizer->SetCard(Cards[i]);
            SpawnedCards.Add(Visualizer);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("ShowCardSelection: Failed to spawn card %d"), i);
        }
    }
}

void UCardManager::HideCardSelection()
{
    for (ACardVisualizer* Visualizer : SpawnedCards)
    {
        if (Visualizer)
        {
            Visualizer->Destroy();
        }
    }
    SpawnedCards.Empty();
}

void UCardManager::SelectCard(UCard* Card)
{
    if (UUpgradeManager* UpgradeManager = GetWorld()->GetSubsystem<UUpgradeManager>())
    {
        UpgradeManager->AddCard(Card);
    }

    // Скрыть карты после выбора
    HideCardSelection();
}