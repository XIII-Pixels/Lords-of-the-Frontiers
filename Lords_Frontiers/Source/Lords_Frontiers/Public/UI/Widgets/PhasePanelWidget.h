#pragma once

#include "Core/GameLoop/GameLoopManager.h"

#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "CoreMinimal.h"

#include "PhasePanelWidget.generated.h"

class UStageProgressWidget;

UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UPhasePanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> TextDay;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> TextDayCurrent;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> TextDayTotal;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UStageProgressWidget> StageProgressBar;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleWaveChanged( int32 CurrentWave, int32 TotalWaves );

	UFUNCTION()
	void HandleTurnChanged( int32 CurrentTurn, int32 MaxTurns );

	UFUNCTION()
	void HandlePhaseChanged( EGameLoopPhase OldPhase, EGameLoopPhase NewPhase );

	void UpdateDayText();
};
