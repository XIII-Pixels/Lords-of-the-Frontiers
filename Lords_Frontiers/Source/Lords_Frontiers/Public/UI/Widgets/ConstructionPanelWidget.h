#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "CoreMinimal.h"

#include "ConstructionPanelWidget.generated.h"

class UWidgetAnimation;

UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UConstructionPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UButton> ButtonCancel;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> HintText;

	UPROPERTY( Transient, meta = ( BindWidgetAnimOptional ) )
	TObjectPtr<UWidgetAnimation> ShowAnim;

	UPROPERTY( Transient, meta = ( BindWidgetAnimOptional ) )
	TObjectPtr<UWidgetAnimation> HideAnim;

	void SetPanelVisible( bool bVisible );

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void OnCancelClicked();

	UFUNCTION()
	void OnVisibilityAnimFinished();

	void PlayVisibilityAnim( bool bVisible );

	bool bWantsVisible_ = false;
};
