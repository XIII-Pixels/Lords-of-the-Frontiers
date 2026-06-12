#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "TextButtonWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnTextButtonClicked );

/**
 * Reusable text button: a UButton with a localizable label.
 * The label is resolved from ST_GameStrings via LabelKey, with plain LabelText as a fallback.
 * Used for the settings OK button and other simple dialog buttons.
 */
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UTextButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY( BlueprintReadOnly, Category = "TextButton", meta = ( BindWidget ) )
	TObjectPtr<UButton> RootButton;

	UPROPERTY( BlueprintReadOnly, Category = "TextButton", meta = ( BindWidget ) )
	TObjectPtr<UTextBlock> ButtonText;

	/** Key in ST_GameStrings. Takes priority over LabelText. */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "TextButton", meta = ( ExposeOnSpawn = "true" ) )
	FName LabelKey;

	/** Plain label used when LabelKey is None. */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "TextButton", meta = ( ExposeOnSpawn = "true" ) )
	FText LabelText;

	UPROPERTY( BlueprintAssignable, Category = "TextButton" )
	FOnTextButtonClicked OnClicked;

	UFUNCTION( BlueprintCallable, Category = "TextButton" )
	void SetLabelKey( FName key );

	UFUNCTION( BlueprintCallable, Category = "TextButton" )
	void SetLabelText( FText text );

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION() void HandleClicked();

	void ApplyLabel();
};
