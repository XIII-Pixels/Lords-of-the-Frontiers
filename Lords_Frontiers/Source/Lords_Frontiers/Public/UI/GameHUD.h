#pragma once
#include "Core/GameLoop/GameLoopManager.h"
#include "Core/GameSessionController.h"
#include "Core/Selection/SelectionManagerComponent.h"
#include "Lords_Frontiers/Public/UI/Widgets/ResourceItemWidget.h"
#include "Lords_Frontiers/Public/Units/Unit.h"
#include "Lords_Frontiers/Public/Waves/EnemyGroupSpawnPoint.h"
#include "Lords_Frontiers/Public/Waves/WaveManager.h"
#include "Resources/GameResource.h"
#include "UI/BonusNeighborhood/BonusIconWidget.h"
#include "UI/InfoWaves/WaveInfoPanelWidget.h"
#include "UI/Widgets/BuildingTooltipWidget.h"
#include "UI/Widgets/CombatTimerWidget.h"
#include "UI/Widgets/ConstructionPanelWidget.h"
#include "UI/Widgets/GameStateOverlayWidget.h"
#include "UI/Widgets/HUDBuildingPanelWidget.h"
#include "UI/Widgets/HUDResourcePanelWidget.h"
#include "UI/Widgets/PhasePanelWidget.h"
#include "UI/Widgets/SelectedBuildingPanelWidget.h"
#include "UI/Widgets/EnemyTooltipWidget.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "CoreMinimal.h"
#include "Sound/AudioTags.h"

#include "GameHUD.generated.h"

class ABuilding;
class UGameStateOverlayWidget;
class UHealthBarWidget;
class UCursorAnimationConfig;
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UGameHUDWidget : public UUserWidget, public IAudioEventSource
{
	GENERATED_BODY()

public:
	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UHUDBuildingPanelWidget> BuildingPanel;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UConstructionPanelWidget> ConstructionPanel;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<USelectedBuildingPanelWidget> EconomySelectedPanel;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<USelectedBuildingPanelWidget> DefensiveSelectedPanel;

	UPROPERTY( meta = ( BindWidget ) )
	UImage* Strokestatus;

	UPROPERTY( EditAnywhere, Category = "Settings|UI|Status" )
	UTexture2D* BackMorningTexture;

	UPROPERTY( EditAnywhere, Category = "Settings|UI|Status" )
	UTexture2D* BackEveningTexture;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UPhasePanelWidget> PhasePanel;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UHUDResourcePanelWidget> ResourcePanel;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UCombatTimerWidget> CombatTimerPanel;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UCanvasPanel> BonusIconCanvas;

	UFUNCTION()
	void HandleBonusPreviewUpdated( const TArray<FBonusIconData>& BonusIcons );

	void ClearBonusIcons();

	TArray<FBonusIconData> CachedBonusData_;

	void UpdateBonusIconPositions();

	UPROPERTY( EditAnywhere, Category = "Settings|Bonus" )
	TSubclassOf<UBonusIconWidget> BonusIconWidgetClass;

	UPROPERTY( EditAnywhere, Category = "Settings|UI|CursorAnim" )
	TObjectPtr<UCursorAnimationConfig> CursorAnimConfig;

	UPROPERTY()
	TArray<TObjectPtr<UBonusIconWidget>> ActiveBonusIcons_;
	TArray<FVector> ActiveBonusWorldPositions_;

	UFUNCTION()
	void UpdateWaveInfo();

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|UI|Overlay" )
	TSubclassOf<UGameStateOverlayWidget> WinWidgetClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|UI|Overlay" )
	TSubclassOf<UGameStateOverlayWidget> LoseWidgetClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|UI|Overlay" )
	TSubclassOf<UGameStateOverlayWidget> PauseWidgetClass;

	UPROPERTY()
	TObjectPtr<UGameStateOverlayWidget> ActiveOverlay;

	UFUNCTION( BlueprintCallable )
	void TogglePauseMenu();

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UHorizontalBox> BossBarsContainer;

	bool AddBossBar( UHealthBarWidget* bar );

	void RemoveBossBar( UHealthBarWidget* bar );

	UFUNCTION()
	void InitSelectionManager( USelectionManagerComponent* InSelectionManager );

	UFUNCTION( BlueprintCallable )
	void HideTooltipForEnemy();

	UFUNCTION( BlueprintCallable )
	void HideTooltipForBuilding();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick( const FGeometry& MyGeometry, float InDeltaTime ) override;

	UFUNCTION()
	void HandlePhaseChanged( EGameLoopPhase OldPhase, EGameLoopPhase NewPhase );

	UFUNCTION()
	void HandlePlacingStarted();

	UFUNCTION()
	void HandlePlacingEnded();

	UFUNCTION()
	void HandleBuildingButtonHovered();

	UFUNCTION()
	void HandleTurnChanged( int32 CurrentTurn, int32 MaxTurns );

	void UpdateStatusText();

	virtual FOnAudioEvent& GetOnAudioEvent() override
	{
		return OnAudioEvent_;
	}

	UPROPERTY( EditAnywhere, Category = "Settings|UI|Tooltip" )
	TSubclassOf<UEnemyTooltipWidget> EnemyTooltipClass;

	UPROPERTY() TObjectPtr<UEnemyTooltipWidget> ActiveEnemyTooltip;

	UPROPERTY( EditAnywhere, Category = "Settings|UI|WaveInfo" )
	TSubclassOf<UWaveInfoPanelWidget> WavePanelClass;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UWaveInfoPanelWidget> WaveInfoPanel;

	UPROPERTY()
	TObjectPtr<UWaveInfoPanelWidget> ActiveWavePanel;

	UFUNCTION()
	void HandleGameEnded( EGameResult Result );

	void UpdateExtraButtonsVisibility();

	UFUNCTION()
	void HandleSelectionChanged();

	UPROPERTY()
	TObjectPtr<USelectionManagerComponent> SelectionManager;

	FOnAudioEvent OnAudioEvent_;
};
