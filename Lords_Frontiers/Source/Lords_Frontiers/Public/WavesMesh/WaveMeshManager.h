#pragma once

#include "Core/GameLoopManager.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "WaveMeshManager.generated.h"

class UCoreManager;

USTRUCT( BlueprintType )
struct FWaveMeshEntry
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Wave Mesh" )
	FString Label;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Wave Mesh", meta = ( ClampMin = "1" ) )
	int32 ShowOnWave = 1;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Wave Mesh", meta = ( ClampMin = "-1" ) )
	int32 HideAfterWave = -1;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Wave Mesh" )
	bool bShowDuringBuildPhase = true;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Wave Mesh" )
	bool bHideOnCombatEnd = true;


	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Wave Mesh" )
	bool bDisableCollisionWhenHidden = true;


	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Wave Mesh" )
	TArray<AActor*> Actors;
};


UCLASS( BlueprintType, Blueprintable, meta = ( DisplayName = "Wave Mesh Manager" ) )
class LORDS_FRONTIERS_API AWaveMeshManager : public AActor
{
	GENERATED_BODY()

public:
	AWaveMeshManager();

	virtual void BeginPlay() override;
	virtual void EndPlay( const EEndPlayReason::Type endPlayReason ) override;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Wave Mesh Config", meta = ( TitleProperty = "Label" ) )
	TArray<FWaveMeshEntry> WaveEntries;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Wave Mesh Config" )
	bool bHideAllOnStart = true;

	UFUNCTION( BlueprintCallable, Category = "Wave Mesh" )
	void ForceRefreshVisibility();

	UFUNCTION( BlueprintCallable, Category = "Wave Mesh|Debug" )
	void LogAllEntries() const;

protected:

	UFUNCTION()
	void HandlePhaseChanged( EGameLoopPhase OldPhase, EGameLoopPhase NewPhase );

	UFUNCTION()
	void HandleWaveChanged( int32 CurrentWave, int32 TotalWaves );

	UFUNCTION()
	void HandleSystemsReady();

private:
	void BindToGameLoop();

	void UnbindFromGameLoop();

	void HideAllManagedActors();

	void EvaluateAllEntries( int32 currentWave, EGameLoopPhase currentPhase );

	bool ShouldEntryBeVisible( const FWaveMeshEntry& entry, int32 currentWave, EGameLoopPhase currentPhase ) const;

	void SetActorVisible( AActor* actor, bool bVisible, bool bDisableCollision );

	UPROPERTY()
	TWeakObjectPtr<UGameLoopManager> GameLoopManager_;

	int32 CachedWave_ = 0;

	bool bIsBound_ = false;
};