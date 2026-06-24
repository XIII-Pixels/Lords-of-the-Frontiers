#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "MainMenuButtonWidget.generated.h"

class UButton;
class UImage;
class URetainerBox;
class UTextBlock;

UENUM( BlueprintType )
enum class EMainMenuButtonAction : uint8
{
	None,
	NewGame,
	Settings,
	ExitGame
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnMainMenuButtonClicked, EMainMenuButtonAction, Action );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnMainMenuButtonHovered, EMainMenuButtonAction, Action );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnMainMenuButtonUnhovered, EMainMenuButtonAction, Action );

UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UMainMenuButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY( BlueprintReadOnly, Category = "MainMenuButton", meta = ( BindWidget ) )
	TObjectPtr<UButton> RootButton;

	UPROPERTY( BlueprintReadOnly, Category = "MainMenuButton", meta = ( BindWidget ) )
	TObjectPtr<UTextBlock> ButtonText;

	UPROPERTY( BlueprintReadOnly, Category = "MainMenuButton", meta = ( BindWidgetOptional ) )
	TObjectPtr<UImage> GlowImage;

	UPROPERTY( BlueprintReadOnly, Category = "MainMenuButton", meta = ( BindWidgetOptional ) )
	TObjectPtr<URetainerBox> GlowRetainer;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "MainMenuButton", meta = ( ExposeOnSpawn = "true" ) )
	EMainMenuButtonAction Action = EMainMenuButtonAction::None;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "MainMenuButton", meta = ( ExposeOnSpawn = "true" ) )
	FName LabelKey;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "MainMenuButton", meta = ( ExposeOnSpawn = "true" ) )
	FText LabelText;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "MainMenuButton|Text" )
	FLinearColor TextColorIdle = FLinearColor::White;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "MainMenuButton|Text" )
	FLinearColor TextColorHovered = FLinearColor( 1.0f, 0.95f, 0.8f, 1.0f );

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "MainMenuButton|Glow" )
	FLinearColor GlowColorIdle = FLinearColor( 1.0f, 0.85f, 0.4f, 0.25f );

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "MainMenuButton|Glow" )
	FLinearColor GlowColorHovered = FLinearColor( 1.0f, 0.9f, 0.55f, 1.0f );

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "MainMenuButton|Glow", meta = ( ClampMin = "0.0" ) )
	float GlowTransitionTime = 0.15f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "MainMenuButton|Glow" )
	bool bGlowTextOutline = false;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "MainMenuButton|Glow" )
	FName GlowColorParameter = TEXT( "GlowColor" );

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "MainMenuButton|Glow" )
	FName GlowProgressParameter = TEXT( "GlowProgress" );

	UPROPERTY( BlueprintAssignable, Category = "MainMenuButton" )
	FOnMainMenuButtonClicked OnClicked;

	UPROPERTY( BlueprintAssignable, Category = "MainMenuButton" )
	FOnMainMenuButtonHovered OnHovered;

	UPROPERTY( BlueprintAssignable, Category = "MainMenuButton" )
	FOnMainMenuButtonUnhovered OnUnhovered;

	UFUNCTION( BlueprintCallable, Category = "MainMenuButton" )
	void SetLabelKey( FName key );

	UFUNCTION( BlueprintCallable, Category = "MainMenuButton" )
	void SetLabelText( FText text );

	UFUNCTION( BlueprintCallable, Category = "MainMenuButton" )
	void SetAction( EMainMenuButtonAction newAction );

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick( const FGeometry& myGeometry, float inDeltaTime ) override;

	UFUNCTION( BlueprintImplementableEvent, Category = "MainMenuButton" )
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
