#pragma once

#include "Blueprint/UserWidget.h"
#include "Core/Debug/DebugPlayerController.h"
#include "CoreMinimal.h"

#include "GameStateOverlayWidget.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnResumeRequested );
class UButton;

UCLASS( Abstract )
class LORDS_FRONTIERS_API UGameStateOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UPROPERTY( BlueprintAssignable, Category = "Events" )
	FOnResumeRequested OnResumeRequested;

	UFUNCTION()
	void HandleTutorialButtonClicked();

protected:
	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> ButtonMainMenu;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> ButtonRestart;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> ButtonNextLevel;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> ButtonResume;

	UPROPERTY( EditAnywhere, meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> TutorialButton;

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