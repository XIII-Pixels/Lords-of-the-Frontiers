// DebugUIWidget.h

#pragma once

#include "Lords_Frontiers/Public/UI/Widgets/ResourceItemWidget.h"
#include "Lords_Frontiers/Public/Units/Unit.h"
#include "Lords_Frontiers/Public/Waves/EnemyGroupSpawnPoint.h"
#include "Lords_Frontiers/Public/Waves/WaveManager.h"

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "CoreMinimal.h"

#include "DebugUIWidget.generated.h"

class UResourceManager;
class UButton;
class AGridVisualizer;
class ABuildManager;
class ABuilding;
class USelectionManagerComponent;
class ADebugPlayerController;

UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UDebugUIWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> Button1;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> Button2;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> Button3;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> Button4;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> Button9;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> Button7 = nullptr;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> ButtonEnemyWave = nullptr;

	UFUNCTION( BlueprintCallable, Category = "Settings|Selection" )
	void InitSelectionManager( USelectionManagerComponent* InSelectionManager );

	// --- Debug spawn settings (editable in BP) ---

	/** Unit class to spawn when debug button pressed. Set this in BP or defaults.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Debug|Spawn" )
	TSubclassOf<AUnit> EnemyClassToSpawn;

	/** Number of units to spawn */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Debug|Spawn", meta = ( ClampMin = "0" ) )
	int32 SpawnCount = 5;

	/** Interval between individual spawns (seconds). If <= 0 -> instant spawn */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Debug|Spawn", meta = ( ClampMin = "0.0" ) )
	float SpawnInterval = 0.5f;

	/** Distance in front of the camera where the debug spawn point will be placed
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Debug|Spawn" )
	float SpawnPointDistance = 600.0f;

protected:
	void CreateResourceWidgets( UResourceManager* Manager );

	UPROPERTY()
	TObjectPtr<ABuildManager> BuildManager = nullptr;

	UPROPERTY()
	TObjectPtr<USelectionManagerComponent> SelectionManager = nullptr;

	UFUNCTION()
	void OnButton1Clicked();

	UFUNCTION()
	void OnButton2Clicked();

	UFUNCTION()
	void OnButton3Clicked();

	UFUNCTION()
	void OnButton4Clicked();

	UFUNCTION()
	void OnButton9Clicked();

	UFUNCTION()
	void OnButton7Clicked();

	UFUNCTION()
	void OnButtonEnemyWaveClicked();

	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	AEnemyGroupSpawnPoint* SpawnDebugSpawnPoint( const FTransform& transform );

	/** Called by timers to actually spawn a single unit */
	void SpawnEnemyInternal(
	    TWeakObjectPtr<AEnemyGroupSpawnPoint> weakSpawnPoint, TSubclassOf<AUnit> enemyClass, int32 enemyIndex,
	    FTransform fallbackTransform
	);

	/** Clears all active spawn timers */
	void ClearActiveTimers();

	// Cached WaveManager (weak to avoid keeping it alive)
	TWeakObjectPtr<AWaveManager> WaveManagerPtr;

	// Try to find WaveManager in the world and cache it
	void FindAndCacheWaveManager();

	UFUNCTION()
	void HandleAllWavesCompleted();

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UHorizontalBox> ResourceContainer;

	UPROPERTY( EditAnywhere, Category = "Settings|UI" )
	TSubclassOf<UResourceItemWidget> ResourceItemClass;

	UPROPERTY( EditAnywhere, Category = "Settings|UI" )
	TMap<EResourceType, TObjectPtr<UTexture2D>> ResourceIcons;

	UPROPERTY()
	TMap<EResourceType, TObjectPtr<UResourceItemWidget>> ResourceWidgetsMap;

	UResourceManager* GetResourceManager() const;

private:
	UPROPERTY()
	TObjectPtr<AGridVisualizer> GridVisualizer = nullptr;

	UPROPERTY(
	    EditAnywhere, BlueprintReadOnly, Category = "Settings|Buildings", meta = ( AllowPrivateAccess = "true" )
	)
	TSubclassOf<ABuilding> Button2BuildingClass;

	UPROPERTY(
	    EditAnywhere, BlueprintReadOnly, Category = "Settings|Buildings", meta = ( AllowPrivateAccess = "true" )
	)
	TSubclassOf<ABuilding> Button3BuildingClass;

	UPROPERTY(
	    EditAnywhere, BlueprintReadOnly, Category = "Settings|Buildings", meta = ( AllowPrivateAccess = "true" )
	)
	TSubclassOf<ABuilding> Button4BuildingClass;

	bool bExtraButtonsVisible = false;

	void UpdateExtraButtonsVisibility();

	UFUNCTION()
	void HandleSelectionChanged();

	/** Keep timer handles to cancel later */
	TArray<FTimerHandle> ActiveSpawnTimers;

	/** Last created spawn point (optional pointer) */
	UPROPERTY( Transient )
	TObjectPtr<AEnemyGroupSpawnPoint> LastSpawnPoint;

	void StartOrAdvanceWave( AWaveManager* WaveManager );

	UPROPERTY( Transient )
	bool bIsSubscribedToWaveManager = false;
	UFUNCTION()
	void OnResourceChangedHandler( EResourceType Type, int32 NewAmount );
};
