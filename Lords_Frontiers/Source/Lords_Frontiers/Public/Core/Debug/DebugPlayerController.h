// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "UI/Widgets/TutorialWidget.h"
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "DebugPlayerController.generated.h"

class USelectionManagerComponent;
class ABuildManager;
class UCardSelectionHUDComponent;

/**
 *
 */
UCLASS()
class LORDS_FRONTIERS_API ADebugPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADebugPlayerController();

	UFUNCTION( BlueprintCallable, Category = "Settings|Selection" )
	USelectionManagerComponent* GetSelectionManager() const;

	UPROPERTY( EditDefaultsOnly, Category = "UI" )
	TSubclassOf<UTutorialWidget> TutorialWidgetClass;

	UFUNCTION( BlueprintCallable, Category = "UI" )
	void ToggleTutorial();

protected:
	virtual void BeginPlay() override;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Selection|Components" )
	TObjectPtr<USelectionManagerComponent> SelectionManagerComponent_ = nullptr;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Cards" )
	TObjectPtr<UCardSelectionHUDComponent> CardSelectionHUDComponent_ = nullptr;

	virtual void SetupInputComponent() override;

	UPROPERTY()
	TObjectPtr<UTutorialWidget> TutorialWidgetInstance;

	UFUNCTION()
	void HandleTutorialClosed();

private:
	UPROPERTY()
	TObjectPtr<ABuildManager> BuildManager_ = nullptr;

	void HandleLeftClick();
	void HandleRightClick();
	void HandleEscape();

	void EnsureBuildManager();

	void OpenTutorial();
	void CloseTutorial();
};
