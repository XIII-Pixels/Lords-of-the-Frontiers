// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CardVisualizer.generated.h"

class UCard;

UCLASS()
class LORDS_FRONTIERS_API ACardVisualizer : public AActor
{
	GENERATED_BODY()
	
public:
    ACardVisualizer();

    void SetCard(UCard* card);

    UFUNCTION()
    void OnCardClicked(AActor* touchedActor, FKey buttonPressed);

protected:

    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* CardMesh;

    UPROPERTY()
    UMaterialInstanceDynamic* DynamicMaterial;

    UPROPERTY()
    UCard* CurrentCard;

};
