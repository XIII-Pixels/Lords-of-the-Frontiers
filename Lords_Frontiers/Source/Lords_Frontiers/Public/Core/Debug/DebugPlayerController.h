// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Components/WidgetComponent.h"

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

protected:
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Selection|Components" )
	TObjectPtr<USelectionManagerComponent> SelectionManagerComponent_ = nullptr;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Cards" )
	TObjectPtr<UCardSelectionHUDComponent> CardSelectionHUDComponent_ = nullptr;

	virtual void SetupInputComponent() override;

	void OnShowAllHP_Pressed();
	void OnShowAllHP_Released();

private:
	UPROPERTY()
	TObjectPtr<ABuildManager> BuildManager_ = nullptr;

	void HandleLeftClick();
	void HandleRightClick();
	void HandleEscape();

	void EnsureBuildManager();

	// Map to remember previous visibility of WidgetComponents touched
	TMap<TWeakObjectPtr<UWidgetComponent>, bool> ChangedWidgetPrevVisibility_;

	// stack counter (protects against duplicate events / reentrant)
	int32 AltHoldCount_ = 0;
};
