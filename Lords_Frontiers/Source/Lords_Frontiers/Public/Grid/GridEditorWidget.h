#pragma once

#include "EditorUtilityWidget.h"

#include "CoreMinimal.h"

#include "GridEditorWidget.generated.h"

class AGridManager;
class ABuilding;
struct FGridCell;
class UUniformGridPanel;
class UScrollBox;
class UBorder;
class UButton;
class UTextBlock;
class UCheckBox;
class USpinBox;
class UVerticalBox;
class UHorizontalBox;
class USizeBox;
class UComboBoxString;

class UGridEditorWidget;

UCLASS()
class UGridCellClickHandler : public UObject
{
	GENERATED_BODY()

public:
	int32 X = 0;
	int32 Y = 0;

	UPROPERTY()
	TObjectPtr<UGridEditorWidget> Owner = nullptr;

	UFUNCTION()
	void OnClicked();
};

UCLASS()
class UGridEditorWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, Category = "Settings|Grid Editor" )
	void RefreshGrid();

	void SelectCell( const int32 x, const int32 y );

	UFUNCTION( BlueprintCallable, Category = "Settings|Grid Editor" )
	void FindGridManager();

protected:
	virtual void NativeConstruct() override;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Grid Editor", meta = ( ExposeOnSpawn = "true" ) )
	TObjectPtr<AGridManager> GridManager_ = nullptr;

	UPROPERTY( EditAnywhere, Category = "Settings|Grid Editor" )
	float CellButtonSize_ = 42.0f;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UScrollBox> GridScrollBox_ = nullptr;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UVerticalBox> PropertiesPanel_ = nullptr;

private:
	FIntPoint SelectedCoords_ = FIntPoint( -1, -1 );

	UPROPERTY()
	TObjectPtr<UUniformGridPanel> GridPanel_ = nullptr;

	UPROPERTY()
	TMap<FIntPoint, TObjectPtr<UBorder>> CellBorderMap_;

	UPROPERTY()
	TArray<TObjectPtr<UGridCellClickHandler>> ClickHandlers_;

	UPROPERTY()
	TObjectPtr<UTextBlock> CoordsText_ = nullptr;

	UPROPERTY()
	TObjectPtr<UCheckBox> BuildableCheck_ = nullptr;

	UPROPERTY()
	TObjectPtr<UCheckBox> WalkableCheck_ = nullptr;

	UPROPERTY()
	TObjectPtr<USpinBox> BonusSpinBox_ = nullptr;

	UPROPERTY()
	TObjectPtr<UComboBoxString> OccupantComboBox_ = nullptr;

	UPROPERTY()
	TObjectPtr<UButton> ClearOccupantButton_ = nullptr;

	UPROPERTY()
	TObjectPtr<UTextBlock> OccupantText_ = nullptr;

	UPROPERTY()
	TObjectPtr<UButton> RefreshButton_ = nullptr;

	UPROPERTY()
	TMap<FString, TObjectPtr<ABuilding>> BuildingMap_;

	void BuildGrid();

	void BuildPropertiesPanel();

	void UpdateAllCellColors();
	void UpdateCellColor( const int32 x, const int32 y );
	void UpdatePropertiesPanel();

	void RefreshBuildingList();

	FLinearColor GetColorForCell( const FGridCell& cell, const bool bIsSelected ) const;

	UTextBlock* MakeSectionHeader( const FString& text );

	UHorizontalBox* MakeLabeledRow( const FString& label, UWidget* valueWidget );

	UBorder* MakeSeparator();

	UFUNCTION()
	void OnBuildableChanged( bool bIsChecked );

	UFUNCTION()
	void OnWalkableChanged( bool bIsChecked );

	UFUNCTION()
	void OnBonusValueChanged( float inValue );

	UFUNCTION()
	void OnOccupantSelected( FString selectedItem, ESelectInfo::Type selectionType );

	UFUNCTION()
	void OnClearOccupantClicked();

	UFUNCTION()
	void OnRefreshClicked();
};