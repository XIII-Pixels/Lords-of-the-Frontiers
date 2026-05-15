#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tutorial/TutorialTypes.h"

#include "TutorialSubsystem.generated.h"

class AActor;
class APlayerController;
class AStrategyCamera;
class UGameLoopManager;
class UInputComponent;
class UMaterialInterface;
class UTutorialConfig;
class UUserWidget;
enum class EGameLoopPhase : uint8;

UCLASS()
class LORDS_FRONTIERS_API UTutorialSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UTutorialSubsystem* Get( const UObject* worldContextObject );

	virtual bool ShouldCreateSubsystem( UObject* outer ) const override;
	virtual void Initialize( FSubsystemCollectionBase& collection ) override;
	virtual void Deinitialize() override;

	UFUNCTION( BlueprintCallable, Category = "Tutorial" )
	void StartTutorial( UTutorialConfig* config );

	UFUNCTION( BlueprintCallable, Category = "Tutorial" )
	void StopTutorial();

	UFUNCTION( BlueprintCallable, Category = "Tutorial" )
	void NotifyEndTurnPressed();

	UFUNCTION( BlueprintCallable, Category = "Tutorial" )
	void NotifyUpgradePicked();

	void OnBubbleClickAnywhere();

	ETutorialAdvance GetCurrentStepAdvance() const;

	UFUNCTION( BlueprintPure, Category = "Tutorial" )
	bool IsRunning() const
	{
		return bRunning_;
	}

	UFUNCTION( BlueprintPure, Category = "Tutorial" )
	int32 GetCurrentStepIndex() const
	{
		return CurrentStepIndex_;
	}

private:
	void EnterStep( int32 index );
	void Advance();

	void SpawnStepWidget( const FTutorialStep& step );
	void DestroyStepWidget();

	void ApplyHighlight( const TArray<FName>& tags );
	void ClearHighlight();

	void EnableSkipInputCapture();
	void DisableSkipInputCapture();

	UFUNCTION()
	void HandleSkipInputPressed();

	void EnsureCursorForWidget();

	void ReassertInputCapture();

	UFUNCTION()
	void HandlePhaseChanged( EGameLoopPhase oldPhase, EGameLoopPhase newPhase );

	bool IsTutorialMap() const;
	APlayerController* GetPC() const;
	AStrategyCamera* GetStrategyCamera() const;
	UGameLoopManager* GetGameLoop() const;

	UPROPERTY()
	TObjectPtr<UTutorialConfig> Config_;

	UPROPERTY()
	TObjectPtr<UUserWidget> CurrentWidget_;

	UPROPERTY()
	TObjectPtr<UInputComponent> SkipInputComponent_;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> CachedHighlightMaterial_;

	TArray<TWeakObjectPtr<AActor>> HighlightedActors_;

	TWeakObjectPtr<UGameLoopManager> BoundGameLoop_;

	int32 CurrentStepIndex_ = INDEX_NONE;
	bool bRunning_ = false;

	double LastAdvanceTime_ = -1.0;
};
