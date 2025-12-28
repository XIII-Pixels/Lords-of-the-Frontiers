// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "DebugPlayerController.generated.h"

class USelectionManagerComponent;
class ABuildManager;

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

  protected:
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Selection|Components" )
	TObjectPtr<USelectionManagerComponent> SelectionManagerComponent = nullptr;

	virtual void SetupInputComponent() override;

  private:
	UPROPERTY()
	TObjectPtr<ABuildManager> BuildManager_ = nullptr;

	void HandleLeftClick();
	void HandleRightClick();
	void HandleEscape();
	void TestShowCards();
	void EnsureBuildManager();
};
