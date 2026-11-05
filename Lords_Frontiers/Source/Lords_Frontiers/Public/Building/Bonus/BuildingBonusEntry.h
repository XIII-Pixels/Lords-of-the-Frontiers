#pragma once

#include "EntityStats.h"
#include "Lords_Frontiers/Public/Resources/GameResource.h"

#include "CoreMinimal.h"

#include "BuildingBonusEntry.generated.h"

class ABuilding;
struct FEntityStats;
enum class EStatsType : uint8;
enum class EResourceType : uint8;

/** (Maxim)
 */
UENUM( BlueprintType )
enum class EBonusCategory : uint8
{
	Production,
	Maintenance,
	Stats
};

UENUM( BlueprintType )
enum class EBonusShape : uint8
{
	Cross,
	Square
};

USTRUCT( BlueprintType )
struct FBonusIconData
{
	GENERATED_BODY()

	UPROPERTY()
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY()
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY()
	float Value = 0.0f;

	UPROPERTY()
	EResourceType ResourceType = EResourceType::None;

	UPROPERTY()
	EStatsType StatType = EStatsType::MaxHealth;

	UPROPERTY()
	EBonusCategory Category = EBonusCategory::Production;

	UPROPERTY()
	TObjectPtr<UTexture2D> BuildingIcon = nullptr;

	UPROPERTY()
	FIntPoint CellCoords = FIntPoint( -1, -1 );
};

USTRUCT( BlueprintType )
struct FBuildingBonusEntry
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Bonus" )
	TSubclassOf<ABuilding> SourceBuildingClass;

	UPROPERTY( EditAnywhere )
	EBonusCategory Category = EBonusCategory::Production;

	UPROPERTY( EditAnywhere, meta = ( EditCondition = "Category != EBonusCategory::Stats" ) )
	EResourceType ResourceType = EResourceType::None;

	UPROPERTY( EditAnywhere, meta = ( EditCondition = "Category == EBonusCategory::Stats" ) )
	EStatsType StatType = EStatsType::MaxHealth;

	UPROPERTY( EditAnywhere )
	float Value = 0.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Bonus" )
	EBonusShape Shape = EBonusShape::Cross;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Bonus" )
	int32 Radius = 1;
};