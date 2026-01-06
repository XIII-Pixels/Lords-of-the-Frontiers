// DebugUIWidget.h

#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "CoreMinimal.h"
#include "Lords_Frontiers/Public/ResourceManager/ResourceItemWidget.h"

#include "DebugUIWidget.generated.h"

class UResourceManager;
class UButton;
class AGridVisualizer;
class ABuildManager;
class ABuilding;
class USelectionManagerComponent;
class ADebugPlayerController;

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

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UButton> Button9;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> Button7 = nullptr;

	UFUNCTION( BlueprintCallable, Category = "Settings|Selection" )
	void InitSelectionManager( USelectionManagerComponent* InSelectionManager );

	void CreateResourceWidgets( UResourceManager* Manager );

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
	void OnButton9Clicked();

	UFUNCTION()
	void OnButton7Clicked();

	virtual bool Initialize() override;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UHorizontalBox> ResourceContainer;

	UPROPERTY( EditAnywhere, Category = "Settings|UI" )
	TSubclassOf<UResourceItemWidget> ResourceItemClass;

	UPROPERTY( EditAnywhere, Category = "Settings|UI" )
	TMap<EResourceType, TObjectPtr<UTexture2D>> ResourceIcons;

	UPROPERTY()
	TMap<EResourceType, TObjectPtr<UResourceItemWidget>> ResourceWidgetsMap;

	UResourceManager* GetResourceManager() const;

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

	UFUNCTION()
	void OnResourceChangedHandler( EResourceType Type, int32 NewAmount );
};
