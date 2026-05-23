#pragma once

#include "Components/Image.h"
#include "Containers/Ticker.h"
#include "CoreMinimal.h"

#include "AlphaHitImage.generated.h"

/**
 * UImage variant that toggles its own hit-testability based on the alpha of
 * the pixel currently under the cursor. Mouse clicks pass through transparent
 * regions of the brush texture to whatever widget sits behind, and are
 * captured only over opaque (alpha >= AlphaThreshold) pixels.
 *
 * Implementation: each engine tick the widget samples the cached texture
 * pixel at the cursor position and calls SetVisibility(Visible) or
 * SetVisibility(SelfHitTestInvisible) accordingly. Pixel data is cached on
 * first use by locking the texture's mip0 bulk data.
 *
 * Requirements on the texture used as the brush:
 *   - Compression Settings = "User Interface 2D (RGBA)"
 *     (so mip0 stays as uncompressed B8G8R8A8 and Lock returns raw pixels).
 *   - Mip Gen Settings = "NoMipmaps" (recommended for UI).
 */
UCLASS()
class LORDS_FRONTIERS_API UAlphaHitImage : public UImage
{
	GENERATED_BODY()

public:
	/**
	 * Pixels with alpha at or above this threshold (0..1) are considered
	 * opaque and the widget consumes the click there. Lower threshold
	 * = more area is clickable (including semi-transparent fringes).
	 */
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
