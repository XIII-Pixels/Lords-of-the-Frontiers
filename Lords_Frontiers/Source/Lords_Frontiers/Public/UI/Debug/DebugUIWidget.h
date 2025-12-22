// DebugUIWidget.h

#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "CoreMinimal.h"

#include "DebugUIWidget.generated.h"

class UButton;
class AGridVisualizer;
class ABuildManager;
class ABuilding;
class USelectionManagerComponent;
class ADebugPlayerController;

// * Simple debug UI. Не игровое, только для тестов. */
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UDebugUIWidget : public UUserWidget
{
	GENERATED_BODY()

  public:
	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> Button1;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> Button2;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> Button3;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> Button4;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> Button7 = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button8 = nullptr;

	UFUNCTION( BlueprintCallable, Category = "Settings|Selection" )
	void InitSelectionManager( USelectionManagerComponent* InSelectionManager );

  protected:
	UPROPERTY()
	TObjectPtr<ABuildManager> BuildManager = nullptr;

	UPROPERTY()
	TObjectPtr<USelectionManagerComponent> SelectionManager = nullptr;

	UFUNCTION()
	void OnButton1Clicked();

	UFUNCTION()
	void OnButton2Clicked();

	UFUNCTION()
	void OnButton3Clicked();

	UFUNCTION()
	void OnButton4Clicked();

	UFUNCTION()
	void OnButton7Clicked();

	UFUNCTION()
	void OnButton8Clicked();

	virtual bool Initialize() override;

  private:
	UPROPERTY()
	TObjectPtr<AGridVisualizer> GridVisualizer = nullptr;

	UPROPERTY(
	    EditAnywhere, BlueprintReadOnly, Category = "Settings|Buildings", meta = ( AllowPrivateAccess = "true" )
	)
	TSubclassOf<ABuilding> Button2BuildingClass;

	UPROPERTY(
	    EditAnywhere, BlueprintReadOnly, Category = "Settings|Buildings", meta = ( AllowPrivateAccess = "true" )
	)
	TSubclassOf<ABuilding> Button3BuildingClass;

	UPROPERTY(
	    EditAnywhere, BlueprintReadOnly, Category = "Settings|Buildings", meta = ( AllowPrivateAccess = "true" )
	)
	TSubclassOf<ABuilding> Button4BuildingClass;

	bool bExtraButtonsVisible = false;

	void UpdateExtraButtonsVisibility();

	UFUNCTION()
	void HandleSelectionChanged();
};
