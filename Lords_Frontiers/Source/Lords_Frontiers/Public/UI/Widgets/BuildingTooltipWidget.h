#pragma once

#include "EntityStats.h"
#include "Resources/GameResource.h"

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "BuildingTooltipWidget.generated.h"

class UTextBlock;
class UImage;
class UOverlay;
class UPanelWidget;
class UBuildingUIConfig;
class ABuilding;

UENUM()
enum class ETooltipState : uint8
{
	Hidden,
	DelayShow,
	AnimatingIn,
	HoldFlash,
	FadeFlash,
	Visible,
	DelayHide,
	AnimatingOut
};

// str economy
UCLASS( Abstract )
class LORDS_FRONTIERS_API UBuildingTooltipResourceRow : public UUserWidget
{
	GENERATED_BODY()
public:
	void Setup( int32 Amount, UTexture2D* Icon, FSlateColor TextColor, bool bShowTurnSuffix );

	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UTextBlock> Text_Amount;
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UImage> Img_ResourceIcon;
	UPROPERTY( meta = ( BindWidgetOptional ) ) TObjectPtr<UTextBlock> Text_Suffix;
};

// str health
UCLASS( Abstract )
class LORDS_FRONTIERS_API UBuildingTooltipHealthRow : public UUserWidget
{
	GENERATED_BODY()
public:
	void Setup( UTexture2D* Icon, const FString& HealthValue );

protected:
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UImage> Img_HealthIcon;
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UTextBlock> Text_HealthValue;
};

// str stats
UCLASS( Abstract )
class LORDS_FRONTIERS_API UBuildingTooltipStatRow : public UUserWidget
{
	GENERATED_BODY()
public:
	void Setup( UTexture2D* Icon, const FString& StatName, const FString& Value );

protected:
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UImage> Img_StatIcon;
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UTextBlock> Text_StatName;
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UTextBlock> Text_Value;
};

// str bonus
UCLASS( Abstract )
class LORDS_FRONTIERS_API UBuildingTooltipBonusRow : public UUserWidget
{
	GENERATED_BODY()
public:
	void Setup( UTexture2D* TargetIcon, UTexture2D* SourceIcon, float Value, UTexture2D* ResourceIcon );

protected:
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UImage> Img_TargetBuilding;
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UImage> Img_SourceBuilding;
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UTextBlock> Text_Value;
	UPROPERTY( meta = ( BindWidgetOptional ) ) TObjectPtr<UImage> Img_ResourceIcon;
};

// main widgeth tooltip
UCLASS( Abstract )
class LORDS_FRONTIERS_API UBuildingTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick( const FGeometry& MyGeometry, float InDeltaTime ) override;

	UFUNCTION( BlueprintCallable, Category = "Tooltip" )
	void ShowTooltip( TSubclassOf<ABuilding> BuildingClass, bool bInstantSwitch = false );

	UFUNCTION( BlueprintCallable, Category = "Tooltip" )
	void HideTooltip();

	UFUNCTION( BlueprintCallable, Category = "Tooltip" )
	void ForceHide();

protected:
	UPROPERTY( EditAnywhere, Category = "Settings|Animation" ) float ShowDelay = 0.2f;
	UPROPERTY( EditAnywhere, Category = "Settings|Animation" ) float HideDelay = 0.1f;
	UPROPERTY( EditAnywhere, Category = "Settings|Animation" ) float AnimDuration = 0.25f;

	UPROPERTY( EditAnywhere, Category = "Settings|Animation" ) float FlashHoldDuration = 0.05f;
	UPROPERTY( EditAnywhere, Category = "Settings|Animation" ) float SwitchDelay = 0.05f;

	UPROPERTY( EditAnywhere, Category = "Settings|Animation" ) float SlideOffsetX = -50.0f;
	UPROPERTY( EditAnywhere, Category = "Settings|Animation" ) TObjectPtr<UCurveFloat> AnimationCurve;
	UPROPERTY( EditAnywhere, Category = "Settings|Config" ) TObjectPtr<UBuildingUIConfig> UIConfig;

	// color numbers
	UPROPERTY( EditAnywhere, Category = "Settings|Colors" )
	FSlateColor IncomeColor = FSlateColor( FLinearColor::Green );
	UPROPERTY( EditAnywhere, Category = "Settings|Colors" ) FSlateColor ExpenseColor = FSlateColor( FLinearColor::Red );
	UPROPERTY( EditAnywhere, Category = "Settings|Colors" )
	FSlateColor NeutralColor = FSlateColor( FLinearColor::White );

	// class sample
	UPROPERTY( EditAnywhere, Category = "Settings|SubWidgets" )
	TSubclassOf<UBuildingTooltipResourceRow> ResourceRowClass;
	UPROPERTY( EditAnywhere, Category = "Settings|SubWidgets" ) TSubclassOf<UBuildingTooltipStatRow> StatRowClass;
	UPROPERTY( EditAnywhere, Category = "Settings|SubWidgets" ) TSubclassOf<UBuildingTooltipBonusRow> BonusRowClass;
	UPROPERTY( EditAnywhere, Category = "Settings|SubWidgets" ) TSubclassOf<UBuildingTooltipHealthRow> HealthRowClass;

	// elem main widgeth
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UOverlay> AnimationContainer;
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UImage> WhiteFlash;
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UImage> Img_Icon;
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UTextBlock> Text_Name;
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UTextBlock> Text_Description;

	// containers
	UPROPERTY( meta = ( BindWidgetOptional ) ) TObjectPtr<UPanelWidget> Box_Health;

	UPROPERTY( meta = ( BindWidgetOptional ) ) TObjectPtr<UPanelWidget> Box_Stats;
	UPROPERTY( meta = ( BindWidgetOptional ) ) TObjectPtr<UPanelWidget> Box_Cost;
	UPROPERTY( meta = ( BindWidgetOptional ) ) TObjectPtr<UPanelWidget> Box_Maintenance;
	UPROPERTY( meta = ( BindWidgetOptional ) ) TObjectPtr<UPanelWidget> Box_Production;
	UPROPERTY( meta = ( BindWidgetOptional ) ) TObjectPtr<UPanelWidget> Box_Bonus;

private:
	ETooltipState CurrentState = ETooltipState::Hidden;
	float StateTimer = 0.0f;
	float AnimProgress = 0.0f;
	float FlashProgress = 0.0f;

	UPROPERTY() TSubclassOf<ABuilding> CurrentBuildingClass;
	UPROPERTY() TSubclassOf<ABuilding> PendingBuildingClass;

	void ApplyAnimation();
	void UpdateContent();

	UTexture2D* GetResourceIcon( EResourceType Type );
	UTexture2D* GetStatIcon( EStatsType Type );
};