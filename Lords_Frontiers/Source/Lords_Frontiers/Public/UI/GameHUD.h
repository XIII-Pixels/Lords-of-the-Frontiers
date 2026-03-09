#pragma once
#include "Core/GameLoopManager.h"
#include "Lords_Frontiers/Public/UI/Widgets/ResourceItemWidget.h"
#include "Lords_Frontiers/Public/Units/Unit.h"
#include "Lords_Frontiers/Public/Waves/EnemyGroupSpawnPoint.h"
#include "Lords_Frontiers/Public/Waves/WaveManager.h"
#include "UI/Widgets/BuildingTooltipWidget.h"
#include "UI/BonusNeighborhood/BonusIconWidget.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "CoreMinimal.h"

#include "GameHUD.generated.h"

class ABuilding;
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UGameHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY( meta = ( BindWidget ) )
	UImage* BackForButton;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonRelocateBuilding;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonRemoveBuilding;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonEndTurn;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonEconomyBuilding;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonDefensiveBuildings;

	// ==== economic buildings ====
	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonBuildingWoodenHouse;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonBuildingStrawHouse;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonBuildingFarm;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonBuildingLawnHouse;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonBuildingMagicHouse;

	// ==== defensive buildings ====
	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonBuildingWoodWall;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonBuildingStoneWall;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonBuildingTowerT0;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonBuildingTowerT1;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonBuildingTowerT2;

	// Panels where we will add building cards
	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UPanelWidget> BuildingClassGrid;

	UPROPERTY( meta = ( BindWidget ) )
	UHorizontalBox* EconomyCardBox;

	UPROPERTY( meta = ( BindWidget ) )
	UHorizontalBox* DefensiveCardBox;

	// Editable arrays of widget classes (card widgets) per category
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Buildings|CardClasses" )
	TArray<TSubclassOf<UUserWidget>> EconomyBuildingCardClasses;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Buildings|CardClasses" )
	TArray<TSubclassOf<UUserWidget>> DefensiveBuildingCardClasses;

	UPROPERTY( meta = ( BindWidget ) )
	UImage* Strokestatus;

	UPROPERTY( EditAnywhere, Category = "Settings|UI|Status" )
	UTexture2D* BackMorningTexture;

	UPROPERTY( EditAnywhere, Category = "Settings|UI|Status" )
	UTexture2D* BackEveningTexture;

	// ==== text ====
	UPROPERTY( meta = ( BindWidget ) )
	UTextBlock* TextDay;
	UPROPERTY( meta = ( BindWidget ) )
	UTextBlock* Text_Citizens;

	UPROPERTY( meta = ( BindWidget ) )
	UTextBlock* Text_Gold;

	UPROPERTY( meta = ( BindWidget ) )
	UTextBlock* Text_Food;

	UPROPERTY( meta = ( BindWidget ) )
	UTextBlock* TextTimer;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Buildings" )
	TSubclassOf<ABuilding> WoodenHouseClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Buildings" )
	TSubclassOf<ABuilding> StrawHouseClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Buildings" )
	TSubclassOf<ABuilding> FarmClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Buildings" )
	TSubclassOf<ABuilding> LawnHouseClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Buildings" )
	TSubclassOf<ABuilding> MagicHouseClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Buildings" )
	TSubclassOf<ABuilding> WoodWallClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Buildings" )
	TSubclassOf<ABuilding> StoneWallClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Buildings" )
	TSubclassOf<ABuilding> TowerT0Class;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Buildings" )
	TSubclassOf<ABuilding> TowerT1Class;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Buildings" )
	TSubclassOf<ABuilding> TowerT2Class;

	UPROPERTY( EditAnywhere, Category = "Settings|UI|Buttons" )
	float ActiveButtonLiftOffset = -10.0f;

	UPROPERTY(
	    EditAnywhere, BlueprintReadWrite, Category = "Settings|UI|BonusIcons", meta = ( ClampMin = "0.1", ClampMax = "3.0" )
	)
	float BaseBonusIconScale = 0.5f;

	UPROPERTY( EditAnywhere, Category = "Settings|UI|BonusIcons" )
	float MinBonusIconScale = 0.1f;

	UPROPERTY( EditAnywhere, Category = "Settings|UI|BonusIcons" )
	float MaxBonusIconScale = 2.0f;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UCanvasPanel> BonusIconCanvas;

	UFUNCTION()
	void HandleBonusPreviewUpdated( const TArray<FBonusIconData>& BonusIcons );

	void ClearBonusIcons();

	TArray<FBonusIconData> CachedBonusData_;


	void UpdateBonusIconPositions();

	UPROPERTY( EditAnywhere, Category = "Settings|Bonus" )
	TSubclassOf<UBonusIconWidget> BonusIconWidgetClass;

	TArray<TObjectPtr<UBonusIconWidget>> ActiveBonusIcons_;
	TArray<FVector> ActiveBonusWorldPositions_;


protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick( const FGeometry& MyGeometry, float InDeltaTime ) override;

	void UpdateBuildingUIVisibility();
	void CancelCurrentBuilding();

	bool bShowingEconomyBuildings_ = true;
	// Button handlers
	UFUNCTION()
	void OnRelocateBuildingClicked();

	UFUNCTION()
	void OnRemoveBuildingClicked();

	UFUNCTION()
	void OnDefensiveBuildingsClicked();

	UFUNCTION()
	void OnEconomyBuildingClicked();

	UFUNCTION()
	void OnEndTurnClicked();

	void ShowEconomyBuildings();
	void ShowDefensiveBuildings();
	// GameLoop handlers
	UFUNCTION()
	void HandlePhaseChanged( EGameLoopPhase OldPhase, EGameLoopPhase NewPhase );

	UFUNCTION()
	void HandleTurnChanged( int32 CurrentTurn, int32 MaxTurns );

	UFUNCTION()
	void HandleCombatTimer( float TimeRemaining, float TotalTime );

	UFUNCTION()
	void HandleResourceChanged( EResourceType Type, int32 NewAmount );

	// Update methods
	void UpdateDayText();
	void UpdateStatusText();
	void UpdateResources();
	void UpdateButtonVisibility();

	UFUNCTION()
	void OnBuildWoodenHouseClicked();

	UFUNCTION()
	void OnBuildStrawHouseClicked();

	UFUNCTION()
	void OnBuildFarmClicked();

	UFUNCTION()
	void OnBuildLawnHouseClicked();

	UFUNCTION()
	void OnBuildMagicHouseClicked();

	UFUNCTION()
	void OnBuildWoodWallClicked();

	UFUNCTION()
	void OnBuildStoneWallClicked();

	UFUNCTION()
	void OnBuildTowerT0Clicked();

	UFUNCTION()
	void OnBuildTowerT1Clicked();

	UFUNCTION()
	void OnBuildTowerT2Clicked();

	void UpdateCategoryButtonsVisual();

	void StartBuilding( TSubclassOf<ABuilding> BuildingClass );

	void UpdateAllBuildingButtons();

	void UpdateButtonAvailability( UButton* button, TSubclassOf<ABuilding> buildingClass );

	UPROPERTY( EditAnywhere, Category = "Settings|UI|Visuals" )
	FLinearColor AffordableColor = FLinearColor::White;

	UPROPERTY( EditAnywhere, Category = "Settings|UI|Visuals" )
	FLinearColor TooExpensiveColor = FLinearColor( 0.3f, 0.3f, 0.3f, 1.0f );

	UPROPERTY( EditAnywhere, Category = "Settings|UI|Tooltip" )
	TSubclassOf<UBuildingTooltipWidget> TooltipClass;

	UPROPERTY() TObjectPtr<UBuildingTooltipWidget> ActiveTooltip;

	FTimerHandle TooltipTimerHandle;
	TSubclassOf<ABuilding> PendingBuildingClass;

	UFUNCTION() void OnHoverWoodenHouse()
	{
		StartTooltipTimer( WoodenHouseClass );
	}
	UFUNCTION() void OnHoverStrawHouse()
	{
		StartTooltipTimer( StrawHouseClass );
	}
	UFUNCTION() void OnHoverFarm()
	{
		StartTooltipTimer( FarmClass );
	}
	UFUNCTION() void OnHoverLawnHouse()
	{
		StartTooltipTimer( LawnHouseClass );
	}
	UFUNCTION() void OnHoverMagicHouse()
	{
		StartTooltipTimer( MagicHouseClass );
	}
	UFUNCTION() void OnHoverWoodWall()
	{
		StartTooltipTimer( WoodWallClass );
	}
	UFUNCTION() void OnHoverStoneWall()
	{
		StartTooltipTimer( StoneWallClass );
	}
	UFUNCTION() void OnHoverTowerT0()
	{
		StartTooltipTimer( TowerT0Class );
	}
	UFUNCTION() void OnHoverTowerT1()
	{
		StartTooltipTimer( TowerT1Class );
	}
	UFUNCTION() void OnHoverTowerT2()
	{
		StartTooltipTimer( TowerT2Class );
	}

	UFUNCTION() void OnBuildingUnhovered();
	UFUNCTION() void ShowTooltipInternal();

	void StartTooltipTimer( TSubclassOf<ABuilding> buildingClass );
};