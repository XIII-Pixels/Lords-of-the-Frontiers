#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "CardIconStripWidget.generated.h"

class UHorizontalBox;
class UImage;
class UTexture2D;

UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UCardIconStripWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, Category = "Card|Feedback" )
	int32 AddIcon( UTexture2D* icon );

	UFUNCTION( BlueprintCallable, Category = "Card|Feedback" )
	void RemoveIcon( int32 slotId );

	UFUNCTION( BlueprintCallable, Category = "Card|Feedback" )
	void ClearAll();

	UFUNCTION( BlueprintPure, Category = "Card|Feedback" )
	int32 GetSlotCount() const
	{
		return Slots_.Num();
	}

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Layout" )
	FVector2D IconSize = FVector2D( 64.f, 64.f );

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Layout" )
	FMargin SlotPadding = FMargin( 4.f, 0.f );

protected:
	UPROPERTY( BlueprintReadOnly, meta = ( BindWidget ) )
	TObjectPtr<UHorizontalBox> IconBox;

	UPROPERTY()
	TMap<int32, TObjectPtr<UImage>> Slots_;

	int32 NextSlotId_ = 1;
};
