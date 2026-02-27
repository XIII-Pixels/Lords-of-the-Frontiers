// MainGameMode.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Core/Subsystems/ProjectilePoolSubsystem/ProjectilePoolSubsystem.h"

#include "MainGameMode.generated.h"

class UGameLoopConfig;
class UUserWidget;
class UCardPoolConfig;

UCLASS()
class LORDS_FRONTIERS_API AMainGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMainGameMode();

	virtual void StartPlay() override;

protected:
	UPROPERTY( EditDefaultsOnly, Category = "Settings|Pools" )
	TArray<FPoolWarmupConfig> ProjectilePoolConfigs;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Config" )
	TObjectPtr<UGameLoopConfig> GameLoopConfig;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Cards" )
	TObjectPtr<UCardPoolConfig> CardPoolConfig;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|UI" )
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> HUDWidget;

private:
	void InitializeGameSystems();
	void InitializeCardSystem();
	void CreateHUD();
	void SetupCamera();
};