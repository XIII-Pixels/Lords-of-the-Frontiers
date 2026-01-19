#pragma once
#include "Lords_Frontiers/Public/UI/Widgets/ResourceItemWidget.h"
#include "Lords_Frontiers/Public/Units/Unit.h"
#include "Lords_Frontiers/Public/Waves/EnemyGroupSpawnPoint.h"
#include "Lords_Frontiers/Public/Waves/WaveManager.h"

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "CoreMinimal.h"

#include "GameHUD.generated.h"

UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UGameHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY ( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonRelocateBuilding;

	UPROPERTY ( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonRemoveBuilding;

	UPROPERTY ( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonEndTurn;

	UPROPERTY ( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonEconomyBuilding;

	UPROPERTY ( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonDefensiveBuildings;

	UPROPERTY ( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonClassBuilding3;

	UPROPERTY ( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonClassBuilding4;


	// ==== economic buildings ====
	UPROPERTY ( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonBuildingWoodenHouse;

	UPROPERTY ( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonBuildingStrawHouse;

	UPROPERTY ( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonBuildingFarm;

	UPROPERTY ( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonBuildingLawnHouse;   // Dom s polyanoy

	UPROPERTY ( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonBuildingMagicHouse;

	// ==== defensive buildings ====
	UPROPERTY ( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonBuildingWall;

	UPROPERTY ( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonBuildingTowerT0;    // rapid attack

	UPROPERTY ( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonBuildingTowerT1;

	UPROPERTY ( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonBuildingTowerT2;    //AOE

	// Panels where we will add building cards
	UPROPERTY ( meta = ( BindWidget ) )
	TObjectPtr<UPanelWidget> BuildingClassGrid;    // container that lists categories

	UPROPERTY ( meta = ( BindWidget ) )  // card boxes
	UHorizontalBox* EconomyCardBox;

	UPROPERTY ( meta = ( BindWidget ) )
	UHorizontalBox* DefensiveCardBox;

	// Editable arrays of widget classes (card widgets) per category. Set in editor
	UPROPERTY ( EditAnywhere, BlueprintReadWrite, Category = "Buildings|CardClasses" )
	TArray<TSubclassOf<UUserWidget>> EconomyBuildingCardClasses;

	UPROPERTY ( EditAnywhere, BlueprintReadWrite, Category = "Buildings|CardClasses" )
	TArray<TSubclassOf<UUserWidget>> DefensiveBuildingCardClasses;

	// add more categories arrays as needed

protected:
	virtual void NativeConstruct () override;
	virtual void NativeDestruct () override;

	// Button handlers
	UFUNCTION ()
	void OnRelocateBuildingClicked ();

	UFUNCTION ()
	void OnRemoveBuildingClicked ();

	UFUNCTION ()
	void OnDefensiveBuildingsClicked ();

	UFUNCTION ()
	void OnEconomyBuildingClicked ();

	UFUNCTION ()
	void OnEndTurnClicked ();

	void ShowEconomyBuildings ();

	void ShowDefensiveBuildings ();

};