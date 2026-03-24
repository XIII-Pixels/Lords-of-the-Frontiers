#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "UI/Widgets/TutorialPageWidget.h"

#include "TutorialWidget.generated.h"

class UButton;
class UWidgetSwitcher;
class UTutorialPageWidget;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FTutorialWidgetClosedSignature );

UCLASS()
class LORDS_FRONTIERS_API UTutorialWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY( BlueprintAssignable, Category = "Tutorial" )
	FTutorialWidgetClosedSignature OnTutorialClosed;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Tutorial" )
	TSubclassOf<UTutorialPageWidget> PageWidgetClass_;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Tutorial" )
	TArray<FTutorialPageData> Pages_;

	UFUNCTION( BlueprintCallable, Category = "Tutorial" )
	void SetPage( int32 pageIndex );

	UFUNCTION( BlueprintCallable, Category = "Tutorial" )
	void NextPage();

	UFUNCTION( BlueprintCallable, Category = "Tutorial" )
	void PrevPage();

	UFUNCTION( BlueprintCallable, Category = "Tutorial" )
	void CloseTutorial();

	UFUNCTION( BlueprintCallable, Category = "Tutorial" )
	void RebuildPages();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnPreviewKeyDown( const FGeometry& inGeometry, const FKeyEvent& inKeyEvent ) override;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UButton> CloseButton_;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UButton> BackdropButton_;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UWidgetSwitcher> PagesSwitcher_;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> NextButton_;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> PrevButton_;

private:
	UPROPERTY()
	TArray<TObjectPtr<UTutorialPageWidget>> BuiltPages_;

	int32 CurrentPageIndex_ = INDEX_NONE;
	bool bPagesBuilt_ = false;

	void BindUI();
	void BuildPages();
	void ActivatePage( int32 newIndex );
	void UpdateNavigationButtons();

	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleBackdropClicked();

	UFUNCTION()
	void HandleNextClicked();

	UFUNCTION()
	void HandlePrevClicked();
};
