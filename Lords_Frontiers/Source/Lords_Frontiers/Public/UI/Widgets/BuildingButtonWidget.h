#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "BuildingButtonWidget.generated.h"

class UButton;
class UImage;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnBuildingButtonClicked );
DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnBuildingButtonHovered );
DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnBuildingButtonUnhovered );

/**
 * Reusable button widget for the HUD building panel.
 *
 * WBP that derives from this class MUST contain:
 *   - a UButton named exactly "RootButton" (handles the actual click / hover hit-test)
 * and MAY contain:
 *   - a UImage named "BuildingIcon" (receives IconTexture brush in NativeConstruct)
 *
 * Hover / scale animations are authored entirely in the WBP (Designer animations
 * driven by the WBP's own Event Graph from RootButton's On Hovered / On Unhovered
 * events). This class only forwards click / hover / unhover to dynamic delegates
 * so the owning panel can keep its AddDynamic-based wiring.
 */
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UBuildingButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY( BlueprintReadOnly, Category = "BuildingButton", meta = ( BindWidget ) )
	TObjectPtr<UButton> RootButton;

	UPROPERTY( BlueprintReadOnly, Category = "BuildingButton", meta = ( BindWidgetOptional ) )
	TObjectPtr<UImage> BuildingIcon;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "BuildingButton", meta = ( ExposeOnSpawn = "true" ) )
	TObjectPtr<UTexture2D> IconTexture;

	UPROPERTY( BlueprintAssignable, Category = "BuildingButton" )
	FOnBuildingButtonClicked OnClicked;

	UPROPERTY( BlueprintAssignable, Category = "BuildingButton" )
	FOnBuildingButtonHovered OnHovered;

	UPROPERTY( BlueprintAssignable, Category = "BuildingButton" )
	FOnBuildingButtonUnhovered OnUnhovered;

	UFUNCTION( BlueprintCallable, Category = "BuildingButton" )
	void SetBackgroundColor( FLinearColor color );

	UFUNCTION( BlueprintPure, Category = "BuildingButton" )
	FLinearColor GetBackgroundColor() const;

	UFUNCTION( BlueprintCallable, Category = "BuildingButton" )
	void SetIconTexture( UTexture2D* texture );

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION() void HandleClicked();
	UFUNCTION() void HandleHovered();
	UFUNCTION() void HandleUnhovered();

	void ApplyIcon();
};
