#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "Templates/SubclassOf.h"

#include "LanguageSwitchWidget.generated.h"

class UButton;
class UImage;
class ULanguageOptionWidget;
class UPanelWidget;
class UTextBlock;
class UWidget;

USTRUCT( BlueprintType )
struct FLanguageOption
{
	GENERATED_BODY()

	/** Culture code understood by UE localization: "ru", "en", ... */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Language" )
	FString Culture;

	/** Label shown next to the flag. Empty = the culture's native name ("Русский", "English"). */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Language" )
	FText DisplayName;

	/** Flag shown on the switch button and in the dropdown list. */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Language" )
	FSlateBrush FlagBrush;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnLanguageChanged, const FString&, Culture );

/**
 * Flag-style language selector: the button shows the current language's flag; clicking it
 * unfolds a dropdown listing the OTHER languages of Languages (the current one stays on
 * the button), clicking a row applies that culture and folds the list. Without a bound
 * list (OptionsPanel/OptionWidgetClass) the button falls back to cycling through Languages.
 * In game builds it sets the process culture and persists it to GameUserSettings.ini;
 * in the editor it drives the game localization preview instead, so only game text is
 * translated and the editor UI keeps its own language.
 */
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API ULanguageSwitchWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	ULanguageSwitchWidget( const FObjectInitializer& objectInitializer );

	/**
	 * Shows the option's flag on the image, or collapses the image (with a log warning)
	 * when the option has no FlagBrush — a stale texture would show the wrong language.
	 * Shared by the switch button and the dropdown rows.
	 */
	static void ApplyFlagBrush( UImage* flagImage, const FLanguageOption& option );

	/** Flag button: toggles the dropdown (or cycles languages when no list is bound). */
	UPROPERTY( BlueprintReadOnly, Category = "LanguageSwitch", meta = ( BindWidget ) )
	TObjectPtr<UButton> RootButton;

	/** Flag of the current language, normally placed inside RootButton. */
	UPROPERTY( BlueprintReadOnly, Category = "LanguageSwitch", meta = ( BindWidgetOptional ) )
	TObjectPtr<UImage> CurrentFlagImage;

	/** Optional name of the current language next to the flag. */
	UPROPERTY( BlueprintReadOnly, Category = "LanguageSwitch", meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> LanguageText;

	/** Container the option rows are spawned into (e.g. a VerticalBox). */
	UPROPERTY( BlueprintReadOnly, Category = "LanguageSwitch", meta = ( BindWidgetOptional ) )
	TObjectPtr<UPanelWidget> OptionsPanel;

	/**
	 * Widget that gets shown/hidden as the dropdown — e.g. a Border wrapping OptionsPanel,
	 * so the list has a background. Falls back to OptionsPanel itself when not bound.
	 */
	UPROPERTY( BlueprintReadOnly, Category = "LanguageSwitch", meta = ( BindWidgetOptional ) )
	TObjectPtr<UWidget> OptionsRoot;

	/** Row widget for the dropdown (WBP based on ULanguageOptionWidget). */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LanguageSwitch" )
	TSubclassOf<ULanguageOptionWidget> OptionWidgetClass;

	/** Cultures available in the dropdown. Defaults to ru / en. */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LanguageSwitch" )
	TArray<FLanguageOption> Languages;

	UPROPERTY( BlueprintAssignable, Category = "LanguageSwitch" )
	FOnLanguageChanged OnLanguageChanged;

	/** Switches to the given culture if it is present in Languages. */
	UFUNCTION( BlueprintCallable, Category = "LanguageSwitch" )
	void SetLanguage( const FString& culture );

	UFUNCTION( BlueprintCallable, Category = "LanguageSwitch" )
	FString GetCurrentCulture() const;

	UFUNCTION( BlueprintCallable, Category = "LanguageSwitch" )
	bool IsListOpen() const;

	UFUNCTION( BlueprintCallable, Category = "LanguageSwitch" )
	void OpenList();

	UFUNCTION( BlueprintCallable, Category = "LanguageSwitch" )
	void CloseList();

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION() void HandleFlagClicked();
	UFUNCTION() void HandleOptionPicked( int32 optionIndex );

	void HandleCultureChanged();
	int32 FindCurrentCultureIndex() const;
	void ApplyOption( int32 index );
	void UpdateCurrentDisplay();
	FText ResolveDisplayName( const FLanguageOption& option ) const;
	void RebuildOptions();
	UWidget* GetListRoot() const;

	UPROPERTY()
	TArray<TObjectPtr<ULanguageOptionWidget>> OptionWidgets_;

	int32 CurrentIndex_ = 0;
	FDelegateHandle CultureChangedHandle_;
};
