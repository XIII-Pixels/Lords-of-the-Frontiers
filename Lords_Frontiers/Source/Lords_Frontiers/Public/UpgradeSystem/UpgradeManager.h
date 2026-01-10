#pragma once

#include "CoreMinimal.h"

#include "Subsystems/WorldSubsystem.h"
#include "UpgradeManager.generated.h"

enum class EBuildingCategory : uint8;
struct FCardModifiers;
class UUpgradePreset;
class UCard;
class UCardSystemSettings;
class UCardSelectionWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUpgradeApplied);

UCLASS()
class LORDS_FRONTIERS_API UUpgradeManager : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    void AddCard(UCard* Card);
    FCardModifiers GetModifiersForCategory(EBuildingCategory Category);

    UPROPERTY(BlueprintAssignable)
    FOnUpgradeApplied OnUpgradeApplied;

private:
    UPROPERTY()
    TArray<UCard*> ActiveCards;

    UPROPERTY()
    UUpgradePreset* DebugPresetCard;
};