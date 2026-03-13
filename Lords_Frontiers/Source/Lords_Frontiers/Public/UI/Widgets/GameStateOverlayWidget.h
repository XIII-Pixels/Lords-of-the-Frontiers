#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "GameStateOverlayWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS( Abstract )
class LORDS_FRONTIERS_API UGameStateOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void SetupWinState();
	void SetupLoseState();
	void SetupPauseState();

protected:
	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UTextBlock> TextTitle;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UButton> BtnMainMenu;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UButton> BtnRestart;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UButton> BtnNextLevel;

	UPROPERTY( meta = ( BindWidget ) )
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