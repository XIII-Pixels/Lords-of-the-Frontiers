#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "TutorialWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UTexture2D;

USTRUCT( BlueprintType )
struct FTutorialStep
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	FText InstructionText;

	// Кадры, экспортированные из GIF в PNG/Texture2D.
	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	TArray<TObjectPtr<UTexture2D>> Frames;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FTutorialWidgetClosedSignature );

UCLASS()
class LORDS_FRONTIERS_API UTutorialWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY( BlueprintAssignable, Category = "Tutorial" )
	FTutorialWidgetClosedSignature OnTutorialClosed;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Tutorial" )
	TArray<FTutorialStep> Steps;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Tutorial" )
	float FramesPerSecond = 12.0f;

	UFUNCTION( BlueprintCallable, Category = "Tutorial" )
	void ShowStep( int32 StepIndex );

	UFUNCTION( BlueprintCallable, Category = "Tutorial" )
	void CloseTutorial();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnPreviewKeyDown( const FGeometry& InGeometry, const FKeyEvent& InKeyEvent ) override;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UButton> CloseButton;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UButton> BackdropButton; // полный экран, ловит клик "в пустоту"

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UImage> TutorialImage;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UTextBlock> TutorialText;

private:
	FTimerHandle FrameTimerHandle;
	int32 CurrentStepIndex = INDEX_NONE;
	int32 CurrentFrameIndex = 0;

	void BindUI();
	void StartPlayback();
	void StopPlayback();
	void AdvanceFrame();

	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleBackdropClicked();
};