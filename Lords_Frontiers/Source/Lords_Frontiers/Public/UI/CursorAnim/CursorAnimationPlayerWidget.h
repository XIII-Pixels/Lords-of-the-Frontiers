#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "CursorAnimationPlayerWidget.generated.h"

struct FCursorAnimationEntry;
class UImage;
class UTexture2D;

UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UCursorAnimationPlayerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam( FOnCursorAnimFinished, UCursorAnimationPlayerWidget* );

	FOnCursorAnimFinished OnFinished;

	void Play( const FCursorAnimationEntry& entry );

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick( const FGeometry& myGeometry, float deltaTime ) override;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UImage> AnimImage;

private:
	UPROPERTY()
	TArray<TObjectPtr<UTexture2D>> Frames_;

	float Elapsed_ = 0.0f;
	float Fps_ = 15.0f;
	int32 CurrentFrame_ = -1;
	bool bPlaying_ = false;
};
