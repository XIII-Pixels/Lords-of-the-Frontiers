#pragma once

#include "Building/Animation/ResourceCollectionAnimData.h"

#include "CoreMinimal.h"

#include "ResourcePopupAnimTypes.generated.h"

class UResourcePopupWidget;

USTRUCT()
struct FResourcePopupBatchEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FVector BuildingWorldLocation = FVector::ZeroVector;

	UPROPERTY()
	FBuildingCollectionAnimData AnimData;

	UPROPERTY()
	float WaveDelay = 0.0f;
};

struct FResourcePopupInstance
{
	TObjectPtr<UResourcePopupWidget> Widget;
	FVector WorldBasePosition = FVector::ZeroVector;
	FVector BaseOffset = FVector::ZeroVector;
	FVector TrajectoryOffset = FVector::ZeroVector;

	float StartDelay = 0.0f;
	float Age = 0.0f;
	bool bFinished = false;

	EResourceType ResourceType = EResourceType::None;
	int32 ResourceAmount = 0;
	bool bCounted = false;
};