// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CardManager.generated.h"

class UCard;
class ACardVisualizer;
class UCardSystemSettings;
/**
 * 
 */
UCLASS()
class LORDS_FRONTIERS_API UCardManager : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    TArray<UCard*> GetRandomCards(int32 count);

    void SelectCard(UCard* card);

    void ShowCardSelection(int32 Count = 3);

    void HideCardSelection();

private:
    UPROPERTY()
    TArray<UCard*> AvailableCards;

    UPROPERTY()
    TArray<ACardVisualizer*> SpawnedCards;

    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<ACardVisualizer> CardVisualizerClass;

    UPROPERTY()
    UCardSystemSettings* Settings;

    void LoadCardsFromFolder();
    void LoadSettings();
};
