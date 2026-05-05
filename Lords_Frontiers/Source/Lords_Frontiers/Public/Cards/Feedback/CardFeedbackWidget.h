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

	void ApplyCameraScale( float zoomAlpha );

	float MinScale() const
	{
		return MinScale_;
	}

	float MaxScale() const
	{
		return MaxScale_;
	}

protected:
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidgetOptional ) )
	TObjectPtr<UImage> IconImage;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Scale" )
	float MinScale_ = 0.5f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Scale" )
	float MaxScale_ = 1.5f;
};
