// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Card.generated.h"

UENUM(BlueprintType)
enum class EBuildingCategory : uint8
{
    All,
    Defensive,
    Resource
};

USTRUCT(BlueprintType)
struct FResourceModifiers
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GoldMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MagicMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FoodMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PeopleMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FCardModifiers
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FireRateMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RangeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HPMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FResourceModifiers Resources;

    FCardModifiers& operator *= (const FCardModifiers & Other)
    {
        DamageMultiplier *= Other.DamageMultiplier;
        FireRateMultiplier *= Other.FireRateMultiplier;
        RangeMultiplier *= Other.RangeMultiplier;
        HPMultiplier *= Other.HPMultiplier;
        Resources.GoldMultiplier *= Other.Resources.GoldMultiplier;
        Resources.MagicMultiplier *= Other.Resources.MagicMultiplier;
        Resources.FoodMultiplier *= Other.Resources.FoodMultiplier;
        Resources.PeopleMultiplier *= Other.Resources.PeopleMultiplier;
        return *this;
    }
};

/**
 * 
 */
UCLASS()
class LORDS_FRONTIERS_API UCard : public UDataAsset
{
	GENERATED_BODY()
	

public:
    const FCardModifiers& GetCardModifiers() const { return CardMod; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Card|CardName")
    FText CardName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Card|SpawnChance")
    float SpawnChance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Card|BuildingCategory")
    EBuildingCategory AffectedCategory = EBuildingCategory::All;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Card|Texture")
    UTexture2D* CardTexture = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Card|StatsCard")
    FCardModifiers CardMod;

protected:

private:


};
