// Fill out your copyright notice in the Description page of Project Settings.

#include "UpgradeSystem/CardVisualizer.h"
#include "UpgradeSystem/Card.h"
#include "UpgradeSystem/CardManager.h"

ACardVisualizer::ACardVisualizer()
{
	CardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CardMesh"));
	RootComponent = CardMesh;
}

// Called when the game starts or when spawned
void ACardVisualizer::BeginPlay()
{
	Super::BeginPlay();
	
	if (CardMesh && CardMesh->GetMaterial(0))
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(CardMesh->GetMaterial(0), this);
		CardMesh->SetMaterial(0, DynamicMaterial);
	}

	// Подписываемся на клик
	OnClicked.AddDynamic(this, &ACardVisualizer::OnCardClicked);

}

void ACardVisualizer::SetCard(UCard* card)
{
	CurrentCard = card;

	if (DynamicMaterial && card && card->CardTexture)
	{
		DynamicMaterial->SetTextureParameterValue(TEXT("CardTexture"), card->CardTexture);
	}
}

void ACardVisualizer::OnCardClicked(AActor* touchedActor, FKey buttonPressed)
{
	if (CurrentCard)
	{
		if (UCardManager* CardManager = GetWorld()->GetSubsystem<UCardManager>())
		{
			CardManager->SelectCard(CurrentCard);
		}
	}
}


