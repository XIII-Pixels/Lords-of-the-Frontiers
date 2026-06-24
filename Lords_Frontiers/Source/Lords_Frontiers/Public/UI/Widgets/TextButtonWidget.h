#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "TextButtonWidget.generated.h"

class UButton;
class UImage;
class URetainerBox;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnTextButtonClicked );
DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnTextButtonHovered );

/**
 * Reusable text button: a UButton with a localizable label and an optional hover-driven
 * glow (the same effect as UMainMenuButtonWidget — see Doc/MainMenu_Glow_Setup.md).
 * The label is resolved from ST_GameStrings via LabelKey, with plain LabelText as a fallback.
 * Glow activates only when GlowImage or GlowRetainer is bound, so plain dialog buttons
 * (e.g. the settings OK button) keep their previous look.
 * Used for the settings OK button, the level-select Back button and other dialog buttons.
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

	/** Soft radial glow behind the caption. Optional — see Doc/MainMenu_Glow_Setup.md (вариант A/B). */
	UPROPERTY( BlueprintReadOnly, Category = "TextButton", meta = ( BindWidgetOptional ) )
	TObjectPtr<UImage> GlowImage;

	/** Retainer box for an Outer-Glow по форме букв — тот же приём, что в главном меню (вариант C). Optional. */
	UPROPERTY( BlueprintReadOnly, Category = "TextButton", meta = ( BindWidgetOptional ) )
	TObjectPtr<URetainerBox> GlowRetainer;

	/** Key in ST_GameStrings. Takes priority over LabelText. */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "TextButton", meta = ( ExposeOnSpawn = "true" ) )
	FName LabelKey;

	/** Plain label used when LabelKey is None. */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "TextButton", meta = ( ExposeOnSpawn = "true" ) )
	FText LabelText;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "TextButton|Text" )
	FLinearColor TextColorIdle = FLinearColor::White;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "TextButton|Text" )
	FLinearColor TextColorHovered = FLinearColor( 1.0f, 0.95f, 0.8f, 1.0f );

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "TextButton|Glow" )
	FLinearColor GlowColorIdle = FLinearColor( 1.0f, 0.85f, 0.4f, 0.25f );

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "TextButton|Glow" )
	FLinearColor GlowColorHovered = FLinearColor( 1.0f, 0.9f, 0.55f, 1.0f );

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "TextButton|Glow", meta = ( ClampMin = "0.0" ) )
	float GlowTransitionTime = 0.15f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "TextButton|Glow" )
	bool bGlowTextOutline = false;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "TextButton|Glow" )
	FName GlowColorParameter = TEXT( "GlowColor" );

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "TextButton|Glow" )
	FName GlowProgressParameter = TEXT( "GlowProgress" );

	UPROPERTY( BlueprintAssignable, Category = "TextButton" )
	FOnTextButtonClicked OnClicked;

	UPROPERTY( BlueprintAssignable, Category = "TextButton" )
	FOnTextButtonHovered OnHovered;

	UFUNCTION( BlueprintCallable, Category = "TextButton" )
	void SetLabelKey( FName key );

	UFUNCTION( BlueprintCallable, Category = "TextButton" )
	void SetLabelText( FText text );

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick( const FGeometry& myGeometry, float inDeltaTime ) override;

	/** Per-frame glow hook for WBP polish (scale/translation/etc.). progress: 0 = idle, 1 = hovered. */
	UFUNCTION( BlueprintImplementableEvent, Category = "TextButton" )
	void OnGlowProgressChanged( float progress );

private:
	UFUNCTION() void HandleClicked();
	UFUNCTION() void HandleHovered();
	UFUNCTION() void HandleUnhovered();

	void ApplyLabel();
	void ApplyGlow();

	bool bIsHovered_ = false;
	float GlowProgress_ = 0.0f;
};
