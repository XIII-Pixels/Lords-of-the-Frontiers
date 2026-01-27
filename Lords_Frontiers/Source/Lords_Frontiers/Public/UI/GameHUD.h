#pragma once
#include "Core/GameLoopManager.h"
#include "Lords_Frontiers/Public/UI/Widgets/ResourceItemWidget.h"
#include "Lords_Frontiers/Public/Units/Unit.h"
#include "Lords_Frontiers/Public/Waves/EnemyGroupSpawnPoint.h"
#include "Lords_Frontiers/Public/Waves/WaveManager.h"

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "CoreMinimal.h"

#include "GameHUD.generated.h"

class ABuilding;
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UGameHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
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

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonClassBuilding3;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonClassBuilding4;

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
	TObjectPtr<UButton> ButtonBuildingWall;

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
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Buildings|CardClasses" )
	TArray<TSubclassOf<UUserWidget>> EconomyBuildingCardClasses;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Buildings|CardClasses" )
	TArray<TSubclassOf<UUserWidget>> DefensiveBuildingCardClasses;

	// ==== text ====
	UPROPERTY( meta = ( BindWidget ) )
	UTextBlock* TextDay;

	UPROPERTY( meta = ( BindWidget ) )
	UTextBlock* TextStatus;

	UPROPERTY( meta = ( BindWidget ) )
	UTextBlock* Text_Citizens;

	UPROPERTY( meta = ( BindWidget ) )
	UTextBlock* Text_Gold;

	UPROPERTY( meta = ( BindWidget ) )
	UTextBlock* Text_Food;

	UPROPERTY( meta = ( BindWidget ) )
	UTextBlock* TextTimer;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Buildings" )
	TSubclassOf<ABuilding> WoodenHouseClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Buildings" )
	TSubclassOf<ABuilding> StrawHouseClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Buildings" )
	TSubclassOf<ABuilding> FarmClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Buildings" )
	TSubclassOf<ABuilding> LawnHouseClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Buildings" )
	TSubclassOf<ABuilding> MagicHouseClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Buildings" )
	TSubclassOf<ABuilding> WallClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Buildings" )
	TSubclassOf<ABuilding> TowerT0Class;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Buildings" )
	TSubclassOf<ABuilding> TowerT1Class;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Buildings" )
	TSubclassOf<ABuilding> TowerT2Class;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void UpdateBuildingUIVisibility();
	void CancelCurrentBuilding();

	bool bShowingEconomyBuildings = true;
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
	void OnBuildWallClicked();

	UFUNCTION()
	void OnBuildTowerT0Clicked();

	UFUNCTION()
	void OnBuildTowerT1Clicked();

	UFUNCTION()
	void OnBuildTowerT2Clicked();

	void StartBuilding( TSubclassOf<ABuilding> BuildingClass );
};