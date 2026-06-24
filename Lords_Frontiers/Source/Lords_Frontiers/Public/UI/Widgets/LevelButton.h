// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Core/Saving/GameSaveData.h"

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "LevelButton.generated.h"

class UImage;
class URetainerBox;
class UTextBlock;
enum class ELevelStatus;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnLevelClicked, int32, LevelIndex );

/** (Gregory-hub)
 * Represents button in level choosing menu. Carries a localized caption and the same
 * hover-driven glow as the main-menu buttons (see UMainMenuButtonWidget / Doc/MainMenu_Glow_Setup.md). */
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API ULevelButton : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleClick();

	void SetStateLocked();
	void SetStateUnlocked();
	void SetStateCompleted();

	int LevelIndex() const
	{
		return LevelIndex_;
	}

	/** Sets the localization key (ST_GameStrings) for the caption. Takes priority over LabelText. */
	UFUNCTION( BlueprintCallable, Category = "LevelButton" )
	void SetLabelKey( FName key );

	UFUNCTION( BlueprintCallable, Category = "LevelButton" )
	void SetLabelText( FText text );

	UPROPERTY( BlueprintAssignable )
	FOnLevelClicked OnClicked;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UButton> Butt;

	UPROPERTY( EditAnywhere, meta = ( BindWidget ) )
	TObjectPtr<UImage> StatusImage;

	/** Caption text. Resolved from LabelKey (ST_GameStrings), with LabelText as fallback. */
	UPROPERTY( BlueprintReadOnly, Category = "LevelButton", meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> ButtonText;

	/** Soft radial glow drawn behind the caption. Optional — see Doc/MainMenu_Glow_Setup.md (вариант A/B). */
	UPROPERTY( BlueprintReadOnly, Category = "LevelButton", meta = ( BindWidgetOptional ) )
	TObjectPtr<UImage> GlowImage;

	/** Retainer box for an Outer-Glow по форме букв — тот же приём, что в главном меню (вариант C). Optional. */
	UPROPERTY( BlueprintReadOnly, Category = "LevelButton", meta = ( BindWidgetOptional ) )
	TObjectPtr<URetainerBox> GlowRetainer;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick( const FGeometry& myGeometry, float inDeltaTime ) override;

	/** Per-frame glow hook for WBP polish (scale/translation/etc.). progress: 0 = idle, 1 = hovered. */
	UFUNCTION( BlueprintImplementableEvent, Category = "LevelButton" )
	void OnGlowProgressChanged( float progress );

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = ( ExposeOnSpawn = true ) )
	int LevelIndex_ = -1;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = ( ExposeOnSpawn = true ) )
	TObjectPtr<UTexture2D> TextureLocked_ = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = ( ExposeOnSpawn = true ) )
	TObjectPtr<UTexture2D> TextureUnlocked_ = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = ( ExposeOnSpawn = true ) )
	TObjectPtr<UTexture2D> TextureCompleted_ = nullptr;

	/** Caption localization key from ST_GameStrings (e.g. LevelSelect.Tutorial). Set on the instance. */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LevelButton|Label", meta = ( ExposeOnSpawn = "true" ) )
	FName LabelKey;

	/** Plain caption used when LabelKey is None. */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LevelButton|Label", meta = ( ExposeOnSpawn = "true" ) )
	FText LabelText;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LevelButton|Text" )
	FLinearColor TextColorIdle = FLinearColor::White;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LevelButton|Text" )
	FLinearColor TextColorHovered = FLinearColor( 1.0f, 0.95f, 0.8f, 1.0f );

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LevelButton|Glow" )
	FLinearColor GlowColorIdle = FLinearColor( 1.0f, 0.85f, 0.4f, 0.25f );

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LevelButton|Glow" )
	FLinearColor GlowColorHovered = FLinearColor( 1.0f, 0.9f, 0.55f, 1.0f );

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LevelButton|Glow", meta = ( ClampMin = "0.0" ) )
	float GlowTransitionTime = 0.15f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LevelButton|Glow" )
	bool bGlowTextOutline = false;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LevelButton|Glow" )
	FName GlowColorParameter = TEXT( "GlowColor" );

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LevelButton|Glow" )
	FName GlowProgressParameter = TEXT( "GlowProgress" );

private:
	UFUNCTION() void HandleHovered();
	UFUNCTION() void HandleUnhovered();

	void ApplyLabel();
	void ApplyGlow();

	bool bIsHovered_ = false;
	float GlowProgress_ = 0.0f;
};
