// MainGameMode.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "MainGameMode.generated.h"

class UGameLoopConfig;
class UUserWidget;

UCLASS()
class LORDS_FRONTIERS_API AMainGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMainGameMode();

	virtual void StartPlay() override;

protected:
	// ========================================================================
	// Конфигурация
	// ========================================================================

	/** Конфиг игрового цикла (Data Asset) */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Config" )
	TObjectPtr<UGameLoopConfig> GameLoopConfig;

	/** Класс HUD виджета */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "UI" )
	TSubclassOf<UUserWidget> HUDWidgetClass;

	// ========================================================================
	// Runtime
	// ========================================================================

	UPROPERTY()
	TObjectPtr<UUserWidget> HUDWidget;

private:
	void InitializeGameSystems();
	void CreateHUD();
	void SetupCamera();
};