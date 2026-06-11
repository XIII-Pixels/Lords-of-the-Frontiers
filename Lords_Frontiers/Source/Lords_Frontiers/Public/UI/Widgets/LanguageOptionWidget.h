#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "UI/Widgets/LanguageSwitchWidget.h"

#include "LanguageOptionWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnLanguageOptionPicked, int32, OptionIndex );

/**
 * Single row of the language dropdown: flag + language name.
 * Created by ULanguageSwitchWidget for every entry of its Languages array.
 */
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API ULanguageOptionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY( BlueprintReadOnly, Category = "LanguageOption", meta = ( BindWidget ) )
	TObjectPtr<UButton> RootButton;

	UPROPERTY( BlueprintReadOnly, Category = "LanguageOption", meta = ( BindWidgetOptional ) )
	TObjectPtr<UImage> FlagImage;

	UPROPERTY( BlueprintReadOnly, Category = "LanguageOption", meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY( BlueprintAssignable, Category = "LanguageOption" )
	FOnLanguageOptionPicked OnPicked;

	/** Fills the row from a language option; called by the switch right after CreateWidget. */
	void Setup( const FLanguageOption& option, const FText& resolvedName, int32 optionIndex );

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION() void HandleClicked();

	int32 OptionIndex_ = INDEX_NONE;
};
