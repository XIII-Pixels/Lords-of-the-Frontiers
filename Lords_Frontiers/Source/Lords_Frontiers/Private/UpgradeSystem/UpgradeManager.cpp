// Fill out your copyright notice in the Description page of Project Settings.


#include "UpgradeSystem/UpgradeManager.h"
#include "UpgradeSystem/UpgradePreset.h"
#include "UpgradeSystem/CardManager.h"
#include "UpgradeSystem/Card.h"
#include "UpgradeSystem/CardSystemSettings.h"

void UUpgradeManager::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (DebugPresetCard)
	{
		for (UCard* Card : DebugPresetCard->CardPreset)
		{
			AddCard(Card);
		}
	}
}

void UUpgradeManager::AddCard(UCard* card)
{

	if (card)
	{
		ActiveCards.Add(card);
	}
}

FCardModifiers UUpgradeManager::GetModifiersForCategory(EBuildingCategory category)
{
	FCardModifiers statsCategoty;

	for (const UCard* card : ActiveCards)
	{
		if (IsValid(card))
		{
			if (card->AffectedCategory == category)
			{
				statsCategoty *= card->CardMod;
			}
		}
	}
	return statsCategoty;
}

void UUpgradeManager::SelectCard(UCard* Card)
{
	if (!Card)
	{
		return;
	}

	AddCard(Card);

	OnUpgradeApplied.Broadcast();

	if (UCardManager* CardManager = GetWorld()->GetSubsystem<UCardManager>())
	{
		CardManager->HideCardSelection();
	}
}