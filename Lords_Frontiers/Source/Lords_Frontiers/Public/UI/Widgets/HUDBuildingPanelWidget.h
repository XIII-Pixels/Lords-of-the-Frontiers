#pragma once

#include "Core/GameLoop/GameLoopManager.h"
#include "Resources/GameResource.h"
#include "Sound/AudioTags.h"

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "CoreMinimal.h"
#include "sound/AudioEvent.h"
#include "sound/AudioEventSource.h"

#include "HUDBuildingPanelWidget.generated.h"

class ABuilding;
class UBuildingButtonWidget;
class UBuildingTooltipWidget;
class UWidgetAnimation;

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnHUDBuildingButtonHovered );

UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UHUDBuildingPanelWidget : public UUserWidget, public IAudioEventSource
{
	GENERATED_BODY()

public:
	// ====== Bound widgets ======
	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UImage> BackForButton;

	UPROPERTY( EditAnywhere, meta = ( BindWidgetOptional ) )
	TObjectPtr<UBuildingButtonWidget> ButtonEconomyBuilding;

	UPROPERTY( EditAnywhere, meta = ( BindWidgetOptional ) )
	TObjectPtr<UBuildingButtonWidget> ButtonDefensiveBuildings;

	// economic buildings
	UPROPERTY( EditAnywhere, meta = ( BindWidgetOptional ) )
	TObjectPtr<UBuildingButtonWidget> ButtonBuildingWoodenHouse;

	UPROPERTY( EditAnywhere, meta = ( BindWidgetOptional ) )
	TObjectPtr<UBuildingButtonWidget> ButtonBuildingStoneHouse;

	UPROPERTY( EditAnywhere, meta = ( BindWidgetOptional ) )
	TObjectPtr<UBuildingButtonWidget> ButtonBuildingGrainField;

	UPROPERTY( EditAnywhere, meta = ( BindWidgetOptional ) )
	TObjectPtr<UBuildingButtonWidget> ButtonBuildingWheatMill;

	UPROPERTY( EditAnywhere, meta = ( BindWidgetOptional ) )
	TObjectPtr<UBuildingButtonWidget> ButtonBuildingMagicHouse;

	// defensive buildings
	UPROPERTY( EditAnywhere, meta = ( BindWidgetOptional ) )
	TObjectPtr<UBuildingButtonWidget> ButtonBuildingWoodWall;

	UPROPERTY( EditAnywhere, meta = ( BindWidgetOptional ) )
	TObjectPtr<UBuildingButtonWidget> ButtonBuildingStoneWall;

	UPROPERTY( EditAnywhere, meta = ( BindWidgetOptional ) )
	TObjectPtr<UBuildingButtonWidget> ButtonBuildingTowerT0;

	UPROPERTY( EditAnywhere, meta = ( BindWidgetOptional ) )
	TObjectPtr<UBuildingButtonWidget> ButtonBuildingTowerT1;

	UPROPERTY( EditAnywhere, meta = ( BindWidgetOptional ) )
	TObjectPtr<UBuildingButtonWidget> ButtonBuildingTowerT2;

	UPROPERTY( EditAnywhere, meta = ( BindWidgetOptional ) )
	TObjectPtr<UBuildingButtonWidget> ButtonBuildingMortira;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UPanelWidget> BuildingClassGrid;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UHorizontalBox> EconomyCardBox;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UHorizontalBox> DefensiveCardBox;

	UPROPERTY( Transient, meta = ( BindWidgetAnimOptional ) )
	TObjectPtr<UWidgetAnimation> ShowAnim;

	UPROPERTY( Transient, meta = ( BindWidgetAnimOptional ) )
	TObjectPtr<UWidgetAnimation> HideAnim;

	// ====== Building classes ======
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Buildings" )
	TSubclassOf<ABuilding> WoodenHouseClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Buildings" )
	TSubclassOf<ABuilding> StoneHouseClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Buildings" )
	TSubclassOf<ABuilding> GrainFieldClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Buildings" )
	TSubclassOf<ABuilding> WheatMillClass;

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

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Buildings" )
	TSubclassOf<ABuilding> TowerMortiraClass;

	// ====== Visuals ======
	UPROPERTY( EditAnywhere, Category = "Settings|UI|Buttons" )
	float ActiveButtonLiftOffset = -10.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|UI|Visuals" )
	FLinearColor AffordableColor = FLinearColor::White;

	UPROPERTY( EditAnywhere, Category = "Settings|UI|Visuals" )
	FLinearColor TooExpensiveColor = FLinearColor( 0.3f, 0.3f, 0.3f, 1.0f );

	// ====== Tooltips ======
	UPROPERTY( EditAnywhere, Category = "Settings|UI|Tooltip" )
	TSubclassOf<UBuildingTooltipWidget> EconomyTooltipClass;

	UPROPERTY( EditAnywhere, Category = "Settings|UI|Tooltip" )
	TSubclassOf<UBuildingTooltipWidget> DefensiveTooltipClass;

	// ====== Public API used by owning HUD ======
	void ShowTooltipForBuilding( TSubclassOf<ABuilding> buildingClass );
	void ShowTooltipForBuilding( const ABuilding* BuildingInstance );

	TSubclassOf<UBuildingTooltipWidget> GetEconomyTooltipClass() const
	{
		return EconomyTooltipClass;
	}

	TSubclassOf<UBuildingTooltipWidget> GetDefensiveTooltipClass() const
	{
		return DefensiveTooltipClass;
	}

	UPROPERTY( BlueprintAssignable )
	FOnHUDBuildingButtonHovered OnBuildingButtonHovered;

	UFUNCTION( BlueprintCallable )
	void HideTooltipForBuilding( bool bAnimate = true );

	void CancelCurrentBuilding();

	void SetBuildingUIVisible( bool bVisible );

	void RefreshAvailability();

	virtual FOnAudioEvent& GetOnAudioEvent() override
	{
		return OnAudioEvent_;
	}

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	bool bShowingEconomyBuildings_ = true;
	bool bIsBuildingLocked = false;
	bool bIsPlacingActive_ = false;
	bool bWantsVisible_ = true;

	UPROPERTY()
	TSubclassOf<ABuilding> LockedBuildingClass;

	UPROPERTY()
	TObjectPtr<UBuildingTooltipWidget> ActiveEconomyTooltip;

	UPROPERTY()
	TObjectPtr<UBuildingTooltipWidget> ActiveDefensiveTooltip;

	UPROPERTY()
	TObjectPtr<UBuildingTooltipWidget> CurrentTooltip;

	FOnAudioEvent OnAudioEvent_;

	// === handlers ===
	UFUNCTION() void OnDefensiveBuildingsClicked();
	UFUNCTION() void OnEconomyBuildingClicked();

	UFUNCTION() void OnBuildWoodenHouseClicked();
	UFUNCTION() void OnBuildStoneHouseClicked();
	UFUNCTION() void OnBuildGrainFieldClicked();
	UFUNCTION() void OnBuildWheatMillClicked();
	UFUNCTION() void OnBuildMagicHouseClicked();
	UFUNCTION() void OnBuildWoodWallClicked();
	UFUNCTION() void OnBuildStoneWallClicked();
	UFUNCTION() void OnBuildTowerT0Clicked();
	UFUNCTION() void OnBuildTowerT1Clicked();
	UFUNCTION() void OnBuildTowerT2Clicked();
	UFUNCTION() void OnBuildTowerMortiraClicked();

	UFUNCTION() void OnHoverWoodenHouse()
	{
		ShowTooltipForBuilding( WoodenHouseClass );
		PlayOnBuildingButtonHoveredSound( ButtonBuildingWoodenHouse );
	}

	UFUNCTION() void OnHoverStoneHouse()
	{
		ShowTooltipForBuilding( StoneHouseClass );
		PlayOnBuildingButtonHoveredSound( ButtonBuildingStoneHouse );
	}

	UFUNCTION() void OnHoverGrainField()
	{
		ShowTooltipForBuilding( GrainFieldClass );
		PlayOnBuildingButtonHoveredSound( ButtonBuildingGrainField );
	}

	UFUNCTION() void OnHoverWheatMill()
	{
		ShowTooltipForBuilding( WheatMillClass );
		PlayOnBuildingButtonHoveredSound( ButtonBuildingWheatMill );
	}

	UFUNCTION() void OnHoverMagicHouse()
	{
		ShowTooltipForBuilding( MagicHouseClass );
		PlayOnBuildingButtonHoveredSound( ButtonBuildingMagicHouse );
	}

	UFUNCTION() void OnHoverWoodWall()
	{
		ShowTooltipForBuilding( WoodWallClass );
		PlayOnBuildingButtonHoveredSound( ButtonBuildingWoodWall );
	}

	UFUNCTION() void OnHoverStoneWall()
	{
		ShowTooltipForBuilding( StoneWallClass );
		PlayOnBuildingButtonHoveredSound( ButtonBuildingStoneWall );
	}

	UFUNCTION() void OnHoverTowerT0()
	{
		ShowTooltipForBuilding( TowerT0Class );
		PlayOnBuildingButtonHoveredSound( ButtonBuildingTowerT0 );
	}

	UFUNCTION() void OnHoverTowerT1()
	{
		ShowTooltipForBuilding( TowerT1Class );
		PlayOnBuildingButtonHoveredSound( ButtonBuildingTowerT1 );
	}

	UFUNCTION() void OnHoverTowerT2()
	{
		ShowTooltipForBuilding( TowerT2Class );
		PlayOnBuildingButtonHoveredSound( ButtonBuildingTowerT2 );
	}

	UFUNCTION() void OnHoverTowerMortira()
	{
		ShowTooltipForBuilding( TowerMortiraClass );
		PlayOnBuildingButtonHoveredSound( ButtonBuildingMortira );
	}

	UFUNCTION()
	void OnHoverEconomyBuilding()
	{
		OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_BUILDINGCATEGORY_HOVERED } );
	}

	UFUNCTION()
	void OnHoverDefensiveBuildings()
	{
		OnAudioEvent_.Broadcast( { AudioTags::SFX_UI_BUTTON_BUILDINGCATEGORY_HOVERED } );
	}

	UFUNCTION() void OnBuildingUnhovered();

	UFUNCTION() void OnPlacingStarted();
	UFUNCTION() void OnPlacingCancelled();

	UFUNCTION() void OnVisibilityAnimFinished();

	UFUNCTION() void HandleResourceChanged( EResourceType Type, int32 NewAmount );

	UFUNCTION() void HandlePhaseChanged( EGameLoopPhase OldPhase, EGameLoopPhase NewPhase );

	// === helpers ===
	void ShowEconomyBuildings();
	void ShowDefensiveBuildings();
	void UpdateCategoryButtonsVisual();
	void UpdateAllBuildingButtons();
	void UpdateButtonAvailability( UBuildingButtonWidget* button, TSubclassOf<ABuilding> buildingClass );
	void StartBuilding( TSubclassOf<ABuilding> BuildingClass );

	void PlayVisibilityAnim( bool bVisible );

	// True when a building is currently selected (its info window is shown).
	bool IsBuildingSelected() const;

	UBuildingTooltipWidget* EnsureTooltipForBuilding( const ABuilding* buildingForTypeCheck );
	void InitializeTooltipWidget(
	    TSubclassOf<UBuildingTooltipWidget> TooltipClass, TObjectPtr<UBuildingTooltipWidget>& OutTooltip
	);

	void PlayOnBuildingButtonClickedSound( const UBuildingButtonWidget* button ) const;
	void PlayOnBuildingButtonHoveredSound( const UBuildingButtonWidget* button ) const;

	bool TrySubscribeManagers();
	void UnsubscribeManagers();
	bool bIsSubscribed_ = false;
};
