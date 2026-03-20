#pragma once

#include "Resources/GameResource.h"

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "BuildingTooltipWidget.generated.h"

class UTextBlock;
class UImage;
class UOverlay;
class UBuildingUIConfig;
class ABuilding;

UENUM()
enum class ETooltipState : uint8
{
	Hidden,
	DelayShow,
	AnimatingIn,
	Visible,
	DelayHide,
	AnimatingOut
};

UCLASS( Abstract )
class LORDS_FRONTIERS_API UBuildingTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick( const FGeometry& MyGeometry, float InDeltaTime ) override;

	UFUNCTION( BlueprintCallable, Category = "Tooltip" )
	void ShowTooltip( TSubclassOf<ABuilding> BuildingClass );

	UFUNCTION( BlueprintCallable, Category = "Tooltip" )
	void HideTooltip();

	UFUNCTION( BlueprintCallable, Category = "Tooltip" )
	void ForceHide();

protected:
	UPROPERTY( EditAnywhere, Category = "Settings|Animation" )
	float ShowDelay = 0.3f;

	UPROPERTY( EditAnywhere, Category = "Settings|Animation" )
	float HideDelay = 0.1f;

	UPROPERTY( EditAnywhere, Category = "Settings|Animation" )
	float AnimDuration = 0.25f;

	UPROPERTY( EditAnywhere, Category = "Settings|Animation" )
	float SlideOffsetX = -50.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|Animation" )
	TObjectPtr<UCurveFloat> AnimationCurve;

	UPROPERTY( EditAnywhere, Category = "Settings|Config" )
	TObjectPtr<UBuildingUIConfig> UIConfig;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UOverlay> AnimationContainer;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UImage> WhiteFlash;

	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UImage> Img_Icon;
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UTextBlock> Text_Name;
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UTextBlock> Text_Description;
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UTextBlock> Text_Stats;
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UTextBlock> Text_Maintenance;
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UTextBlock> Text_Production;
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UTextBlock> Text_Cost;
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UTextBlock> Text_Bonus;

private:
	ETooltipState CurrentState = ETooltipState::Hidden;
	float StateTimer = 0.0f;
	float AnimProgress = 0.0f;

	UPROPERTY()
	TSubclassOf<ABuilding> CurrentBuildingClass;

	void ApplyAnimation();
	void UpdateContent();

	FString FormatResourceString( int32 Gold, int32 Food, int32 Pop, int32 Prog );
};