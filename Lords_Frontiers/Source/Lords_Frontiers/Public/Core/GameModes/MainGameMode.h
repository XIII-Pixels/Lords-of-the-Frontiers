// MainGameMode.h
#pragma once

#include "Core/Subsystems/ProjectilePoolSubsystem/ProjectilePoolSubsystem.h"

#include "CoreMinimal.h"
#include "Core/GameSessionController.h"
#include "GameFramework/GameModeBase.h"

#include "MainGameMode.generated.h"

class UGameLoopConfig;
class UUserWidget;
class UCardPoolConfig;
class UEntityVFXConfig;
class UGameHUDWidget;
class UMatchScoringConfig;
class UMatchResultsWidget;

UCLASS()
class LORDS_FRONTIERS_API AMainGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMainGameMode();

	virtual void StartPlay() override;

	UGameHUDWidget* GetGameHUDWidget() const;

protected:
	UPROPERTY( EditDefaultsOnly, Category = "Settings|Pools" )
	TArray<FPoolWarmupConfig> ProjectilePoolConfigs;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Config" )
	TObjectPtr<UGameLoopConfig> GameLoopConfig;

	UPROPERTY(
	    EditAnywhere, BlueprintReadWrite, Category = "Settings|Config",
	    meta = ( ToolTip = "Per-map overrides. Key = short map name without PIE prefix. Falls back to GameLoopConfig." )
	)
	TMap<FName, TObjectPtr<UGameLoopConfig>> GameLoopConfigByMap;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Cards" )
	TObjectPtr<UCardPoolConfig> CardPoolConfig;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Config", meta = ( DisplayName = "Конфиг очков матча" ) )
	TObjectPtr<UMatchScoringConfig> MatchScoringConfig;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|UI" )
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|UI", meta = ( DisplayName = "Виджет результатов (WBP_MatchResults)" ) )
	TSubclassOf<UMatchResultsWidget> ResultsWidgetClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|UI", meta = ( DisplayName = "Карты бесконечного режима", ToolTip = "Имена уровней (без префиксов), на которых показывается виджет результатов. На остальных уровнях — обычные баннеры Win/Lose." ) )
	TArray<FName> EndlessMapNames = { TEXT( "Level_3_WP" ) };

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|VFX" )
	TObjectPtr<UEntityVFXConfig> EntityVFXConfig;

	UPROPERTY()
	TObjectPtr<UUserWidget> HUDWidget;

private:
	void InitializeGameSystems();
	void InitializeCardSystem();
	void CreateHUD();
	void SetupCamera();

	UGameLoopConfig* SelectGameLoopConfig() const;

	UFUNCTION()
	void HandleGameEnded( EGameResult result );
};