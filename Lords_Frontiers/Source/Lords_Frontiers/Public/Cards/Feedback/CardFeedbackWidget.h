#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "CardFeedbackWidget.generated.h"

class UImage;

UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UCardFeedbackWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, Category = "Card|Feedback" )
	void SetIcon( UTexture2D* icon );

	UFUNCTION( BlueprintImplementableEvent, Category = "Card|Feedback" )
	void OnShow();

	UFUNCTION( BlueprintImplementableEvent, Category = "Card|Feedback" )
	void UpdateAlpha( float alpha );

protected:
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UImage> IconImage;
};
