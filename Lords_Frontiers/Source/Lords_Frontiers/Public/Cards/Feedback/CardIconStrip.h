#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "CardIconStrip.generated.h"

class UWidgetComponent;
class UCardIconStripWidget;
class UTexture2D;

UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API ACardIconStrip : public AActor
{
	GENERATED_BODY()

public:
	ACardIconStrip();

	void ActivateOn( AActor* host );

	void DeactivateToPool();

	int32 AddIconSlot( UTexture2D* icon, float requestedHeightOffset );

	bool RemoveIconSlot( int32 slotId, float releasedHeightOffset );

	int32 GetSlotCount() const;

	bool IsActive() const
	{
		return bIsActive_;
	}

	AActor* GetHost() const
	{
		return HostActor_.Get();
	}

protected:
	virtual void BeginPlay() override;

	void ApplyHeightOffset();
	void EnsureWidgetInitialized();
	void SetDormant( bool bDormant );

	UPROPERTY( VisibleAnywhere, Category = "Settings|Feedback" )
	TObjectPtr<UWidgetComponent> WidgetComponent_;

	UPROPERTY( EditAnywhere, Category = "Settings|Feedback" )
	TSubclassOf<UCardIconStripWidget> WidgetClass_;

	UPROPERTY( EditAnywhere, Category = "Settings|Feedback" )
	FVector2D WidgetDrawSize_ = FVector2D( 384.f, 96.f );

	UPROPERTY()
	TObjectPtr<UCardIconStripWidget> WidgetInstance_;

	UPROPERTY()
	TMap<int32, float> SlotHeightOffsets_;

	float CurrentHeightOffset_ = 0.f;

	bool bIsActive_ = false;

	TWeakObjectPtr<AActor> HostActor_;
};
