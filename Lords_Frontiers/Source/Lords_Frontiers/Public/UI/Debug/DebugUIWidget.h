// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Lords_Frontiers/Public/Unit.h"
#include "Lords_Frontiers/Public/Waves/EnemyGroup.h"
#include "Lords_Frontiers/Public/Waves/EnemyGroupSpawnPoint.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "DebugUIWidget.generated.h"

class AWaveManager;

/** (Gregory-hub)
 * UI class for debugging
 * Do not use this for creating actual game UI */
UCLASS ( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UDebugUIWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY ( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> Button1;

	UPROPERTY ( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> Button2;

	UPROPERTY ( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> Button3;

	UPROPERTY ( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> Button4;

	// --- Debug spawn settings (editable in BP) ---

	/** Unit class to spawn when debug button pressed. Set this in BP or defaults. */
	UPROPERTY ( EditAnywhere, BlueprintReadWrite, Category = "Debug|Spawn" )
	TSubclassOf<AUnit> EnemyClassToSpawn;

	/** Number of units to spawn */
	UPROPERTY ( EditAnywhere, BlueprintReadWrite, Category = "Debug|Spawn", meta = ( ClampMin = "0" ) )
	int32 SpawnCount = 5;

	/** Interval between individual spawns (seconds). If <= 0 -> instant spawn */
	UPROPERTY ( EditAnywhere, BlueprintReadWrite, Category = "Debug|Spawn", meta = ( ClampMin = "0.0" ) )
	float SpawnInterval = 0.5f;

	/** Distance in front of the camera where the debug spawn point will be placed */
	UPROPERTY ( EditAnywhere, BlueprintReadWrite, Category = "Debug|Spawn" )
	float SpawnPointDistance = 600.0f;


protected:
	UFUNCTION()
	void OnButton1Clicked ();

	UFUNCTION()
	void OnButton2Clicked ();

	UFUNCTION()
	void OnButton3Clicked ();

	UFUNCTION()
	void OnButton4Clicked ();

	virtual bool Initialize () override;
	virtual void NativeDestruct () override;

protected:
	// Internal helpers

	/** Spawn a visible EnemyGroupSpawnPoint at the given transform (returns created actor) */
	AEnemyGroupSpawnPoint* SpawnDebugSpawnPoint ( const FTransform& transform );

	/** Called by timers to actually spawn a single unit */
	void SpawnEnemyInternal ( TWeakObjectPtr<AEnemyGroupSpawnPoint> weakSpawnPoint, TSubclassOf<AUnit> enemyClass, int32 enemyIndex, FTransform fallbackTransform );

	/** Clears all active spawn timers */
	void ClearActiveTimers ();

	// Cached WaveManager (weak to avoid keeping it alive)
	TWeakObjectPtr<AWaveManager> WaveManagerPtr;

	// Try to find WaveManager in the world and cache it
	void FindAndCacheWaveManager ();



private:
	/** Keep timer handles to cancel later */
	TArray<FTimerHandle> ActiveSpawnTimers;

	/** Last created spawn point (optional pointer) */
	UPROPERTY ( Transient )
	TObjectPtr<AEnemyGroupSpawnPoint> LastSpawnPoint;

};
