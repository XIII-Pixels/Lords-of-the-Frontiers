#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "TimerManager.h"
#include "TutorialPageWidget.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;

USTRUCT( BlueprintType )
struct FTutorialPageData
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Tutorial" )
	FText Title;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Tutorial" )
	FText Body;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Tutorial" )
	TObjectPtr<UTexture2D> Image = nullptr;


	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Tutorial" )
	TArray<TObjectPtr<UTexture2D>> Frames;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Tutorial" )
	float FPS = 12.0f;
};

UCLASS()
class LORDS_FRONTIERS_API UTutorialPageWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION( BlueprintCallable, Category = "Tutorial" )
	void SetData( const FTutorialPageData& inData );

	UFUNCTION( BlueprintCallable, Category = "Tutorial" )
	void OnPageActivated();

	UFUNCTION( BlueprintCallable, Category = "Tutorial" )
	void OnPageDeactivated();

protected:
	virtual void NativeDestruct() override;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> TitleText_;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UTextBlock> TutorialText_;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UImage> TutorialImage_;

private:
	FTutorialPageData CurrentData_;

	TArray<TObjectPtr<UTexture2D>> CurrentFrames_;
	int32 CurrentFrameIndex_ = 0;
	float CurrentFPS_ = 12.0f;

	bool bPlaying_ = false;
	FTimerHandle FrameTimerHandle_;

	void ApplyStaticContent();
	void StartPlayback();
	void StopPlayback();
	void AdvanceFrame();
};