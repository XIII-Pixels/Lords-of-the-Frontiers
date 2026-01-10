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

void UUpgradeManager::AddCard(UCard* Card)
{
	if (Card)
	{
		ActiveCards.Add(Card);
		OnUpgradeApplied.Broadcast(); 
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
