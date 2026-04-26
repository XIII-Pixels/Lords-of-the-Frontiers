#pragma once

#include "CoreMinimal.h"

#include "CardVisualTypes.generated.h"

class UNiagaraSystem;
class UTexture2D;

UENUM( BlueprintType )
enum class ECardVisualTarget : uint8
{
	None	UMETA( DisplayName = "None (disabled)" ),
	Owner	UMETA( DisplayName = "Owner (building)" ),
	Target	UMETA( DisplayName = "Target (enemy)" ),
	Both	UMETA( DisplayName = "Owner and Target" ),
};

UENUM( BlueprintType )
enum class ECardVisualIconMode : uint8
{
	Popup	UMETA( DisplayName = "Popup (one-shot)" ),
	Sticky	UMETA( DisplayName = "Sticky (while effect active)" ),
};

USTRUCT( BlueprintType )
struct LORDS_FRONTIERS_API FCardNiagaraSpec
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Niagara" )
	TSoftObjectPtr<UNiagaraSystem> System;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Niagara" )
	ECardVisualTarget SpawnOn = ECardVisualTarget::Owner;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Niagara" )
	bool bLoop = false;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Niagara",
		meta = ( ClampMin = "0.0" ) )
	float DelaySeconds = 0.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Niagara" )
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Niagara" )
	bool bAttachToHost = true;
};

USTRUCT( BlueprintType )
struct LORDS_FRONTIERS_API FCardIconSpec
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Icon" )
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Icon" )
	ECardVisualTarget ShowOn = ECardVisualTarget::Owner;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Icon" )
	ECardVisualIconMode Mode = ECardVisualIconMode::Popup;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Icon",
		meta = ( ClampMin = "0.0" ) )
	float DelaySeconds = 0.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Icon|Popup",
		meta = ( ClampMin = "0.05", EditCondition = "Mode == ECardVisualIconMode::Popup", EditConditionHides ) )
	float PopupDurationSeconds = 1.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Icon|Popup",
		meta = ( EditCondition = "Mode == ECardVisualIconMode::Popup", EditConditionHides ) )
	float PopupFloatHeight = 120.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Icon" )
	float BaseHeightOffset = 0.f;
};

USTRUCT( BlueprintType )
struct LORDS_FRONTIERS_API FCardVisualConfig
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Visuals" )
	FCardNiagaraSpec Niagara;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Visuals" )
	FCardIconSpec Icon;
};

USTRUCT( BlueprintType )
struct LORDS_FRONTIERS_API FCardVisualHandle
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Id = INDEX_NONE;

	bool IsValid() const
	{
		return Id != INDEX_NONE;
	}

	void Reset()
	{
		Id = INDEX_NONE;
	}

	friend bool operator==( const FCardVisualHandle& a, const FCardVisualHandle& b )
	{
		return a.Id == b.Id;
	}

	friend uint32 GetTypeHash( const FCardVisualHandle& h )
	{
		return ::GetTypeHash( h.Id );
	}
};
