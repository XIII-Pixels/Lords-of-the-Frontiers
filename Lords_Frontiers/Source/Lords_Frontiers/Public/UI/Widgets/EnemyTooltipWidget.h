#pragma once

#include "UI/Widgets/BuildingTooltipWidget.h"

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "EnemyTooltipWidget.generated.h"

class AUnit;
class UEnemyInfoDataAsset;
class UBuildingUIConfig;
class UOverlay;
class UImage;
class UTextBlock;
class UPanelWidget;
class UCurveFloat;
class UDecalComponent;
class UMaterialInterface;

UCLASS( Abstract )
class LORDS_FRONTIERS_API UEnemyTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick( const FGeometry& myGeometry, float inDeltaTime ) override;

	UFUNCTION( BlueprintCallable, Category = "Tooltip" )
	void ShowTooltip( AUnit* enemyUnit );

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

	UPROPERTY( EditAnywhere, Category = "Settings|Data" )
	TObjectPtr<UEnemyInfoDataAsset> EnemyDataAsset;

	UPROPERTY( EditAnywhere, Category = "Settings|Data" )
	TObjectPtr<UBuildingUIConfig> GlobalUIConfig;

	UPROPERTY( EditAnywhere, Category = "Settings|Classes" )
	TSubclassOf<UBuildingTooltipHealthRow> HealthRowClass;

	UPROPERTY( EditAnywhere, Category = "Settings|Classes" )
	TSubclassOf<UBuildingTooltipStatRow> StatRowClass;

	UPROPERTY( EditAnywhere, Category = "Settings|Visuals" )
	TObjectPtr<UMaterialInterface> RangeIndicatorMaterial;

	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UOverlay> AnimationContainer;
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UImage> WhiteFlash;
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UImage> Img_Icon;
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UTextBlock> Text_Name;
	UPROPERTY( meta = ( BindWidget ) ) TObjectPtr<UTextBlock> Text_Description;

	UPROPERTY( meta = ( BindWidgetOptional ) ) TObjectPtr<UPanelWidget> Box_Health;
	UPROPERTY( meta = ( BindWidgetOptional ) ) TObjectPtr<UPanelWidget> Box_Stats;

private:
	void ApplyAnimation();
	void UpdateContent();
	void RefreshFromInstance();
	UTexture2D* GetStatIcon( EStatsType type ) const;

	ETooltipState CurrentState = ETooltipState::Hidden;
	float StateTimer = 0.0f;
	float AnimProgress = 0.0f;
	float FlashProgress = 0.0f;

	TWeakObjectPtr<AUnit> CurrentEnemy_;
	TWeakObjectPtr<AUnit> PendingEnemy_;

	float InstanceRefreshAccumulator_ = 0.0f;

	struct FEnemySnapshot
	{
		bool bValid = false;
		int32 Health = 0;
		int32 MaxHealth = 0;
		int32 AttackDamage = 0;
		float AttackRange = 0.0f;
		float AttackCooldown = 0.0f;
		float MaxSpeed = 0.0f;
		int32 CritChance = 0;
		int32 CritDamageBonus = 0;

		bool Equals( const FEnemySnapshot& other ) const;
	};

	FEnemySnapshot LastSnapshot_;
	FEnemySnapshot CaptureSnapshot( const AUnit* enemy ) const;

	UPROPERTY() TObjectPtr<UBuildingTooltipHealthRow> CachedHealthRow;
	UPROPERTY() TObjectPtr<UBuildingTooltipStatRow> CachedDamageRow;
	UPROPERTY() TObjectPtr<UBuildingTooltipStatRow> CachedSpeedRow;
	UPROPERTY() TObjectPtr<UBuildingTooltipStatRow> CachedRangeRow;
	UPROPERTY() TObjectPtr<UBuildingTooltipStatRow> CachedCooldownRow;
	UPROPERTY() TObjectPtr<UBuildingTooltipStatRow> CachedCritChanceRow;
	UPROPERTY() TObjectPtr<UBuildingTooltipStatRow> CachedCritDamageRow;

	UPROPERTY( Transient )
	TObjectPtr<UDecalComponent> SpawnedRangeDecal;

	void SpawnRangeIndicator( AUnit* enemy );
	void ClearRangeIndicator();
};