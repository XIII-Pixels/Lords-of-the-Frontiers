#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "GameStateOverlayWidget.generated.h"

class UButton;

UCLASS( Abstract )
class LORDS_FRONTIERS_API UGameStateOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> BtnMainMenu;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> BtnRestart;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> BtnNextLevel;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> BtnResume;

	UFUNCTION()
	void OnMainMenuClicked();

	UFUNCTION()
	void OnRestartClicked();

	UFUNCTION()
	void OnNextLevelClicked();

	UFUNCTION()
	void OnResumeClicked();

	UPROPERTY( EditDefaultsOnly, Category = "Settings" )
	FName mainMenuLevelName = "MainMenu";

	UPROPERTY( EditDefaultsOnly, Category = "Settings" )
	FName nextLevelName = "Level_2";
};