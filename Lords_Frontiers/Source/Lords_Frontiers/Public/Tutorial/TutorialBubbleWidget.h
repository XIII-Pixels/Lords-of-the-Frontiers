#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "TutorialBubbleWidget.generated.h"

class UImage;
class UMaterialInstanceDynamic;
class URichTextBlock;
class UWidget;

UCLASS()
class LORDS_FRONTIERS_API UTutorialBubbleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetBubbleTextKey( FName key );
	void SetHintTextKey( FName key );

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick( const FGeometry& myGeometry, float inDeltaTime ) override;
	virtual FReply NativeOnMouseButtonDown( const FGeometry& geometry, const FPointerEvent& mouseEvent ) override;

	UFUNCTION( BlueprintCallable, Category = "Settings|Tutorial" )
	void SyncHoles();

private:
	bool IsPointInWidget( const UWidget* widget, const FVector2D& screenPosAbs ) const;
	FLinearColor ComputeHoleRectUV( const UWidget* targetWidget ) const;
	void UpdateDimHitTestability();

	void ApplyAllText();
	void ApplyBubbleText();
	void ApplyHintText();
	void ApplyKeyedText( URichTextBlock* block, FName key );
	void HandleTextRevisionChanged();

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UImage> DimImage;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<URichTextBlock> BubbleText;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<URichTextBlock> HintText;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<URichTextBlock> SpeakerText;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<URichTextBlock> CutsceneIntroText;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<URichTextBlock> CutsceneSkipText;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UImage> HoleZone1;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UImage> HoleZone2;

	UPROPERTY( Transient )
	TObjectPtr<UMaterialInstanceDynamic> DimMID_;

	FVector2D LastDimSize_ = FVector2D::ZeroVector;

	FName BubbleTextKey_;
	FName HintTextKey_;

	FDelegateHandle TextRevisionHandle_;
};
