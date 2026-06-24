#pragma once

#include "Components/Image.h"
#include "Containers/Ticker.h"
#include "CoreMinimal.h"

#include "AlphaHitImage.generated.h"

UCLASS()
class LORDS_FRONTIERS_API UAlphaHitImage : public UImage
{
	GENERATED_BODY()

public:

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "AlphaHit", meta = ( ClampMin = "0.0", ClampMax = "1.0" ) )
	float AlphaThreshold = 0.05f;

protected:
	virtual void OnWidgetRebuilt() override;
	virtual void ReleaseSlateResources( bool bReleaseChildren ) override;
	virtual void BeginDestroy() override;

private:
	bool TickUpdate( float deltaTime );
	void UpdateHitVisibility();
	void EnsurePixelCache();
	void StopTicker();

	FTSTicker::FDelegateHandle TickHandle_;

	UPROPERTY()
	TObjectPtr<UTexture2D> CachedTexture_;

	TArray<FColor> CachedPixels_;
	FIntPoint CachedSize_ = FIntPoint::ZeroValue;
};
