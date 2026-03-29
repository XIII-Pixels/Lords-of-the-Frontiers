#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/SizeBox.h"
#include "CoreMinimal.h"

#include "StageProgressWidget.generated.h"

UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UStageProgressWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetTargetProgress( float newTarget );

	void ResetProgress();

	void ResetProgressImmediate();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick( const FGeometry& myGeometry, float inDeltaTime ) override;


	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<USizeBox> ClipPanel;

	UPROPERTY( EditAnywhere, Category = "Settings|StageProgress" )
	float PanelWidth = 512.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|StageProgress" )
	float ClosedWidth = 0.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|StageProgress" )
	float FillDuration = 1.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|StageProgress" )
	float ResetDuration = 0.6f;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UImage> FlagStart;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UImage> FlagMiddle;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UImage> FlagEnd;

	UPROPERTY( EditAnywhere, Category = "Settings|StageProgress|Flags" )
	float FlagStartThreshold = 0.01f;

	UPROPERTY( EditAnywhere, Category = "Settings|StageProgress|Flags" )
	float FlagMiddleThreshold = 0.5f;

	UPROPERTY( EditAnywhere, Category = "Settings|StageProgress|Flags" )
	float FlagEndThreshold = 1.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|StageProgress|Flags" )
	float FlagRiseOffset = 16.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|StageProgress|Flags" )
	float FlagAnimDuration = 0.25f;

private:
	void ApplyBarTranslation( float progress );

	float CurrentProgress_ = 0.0f;
	float TargetProgress_ = 0.0f;
	float StartProgress_ = 0.0f;
	float Elapsed_ = 0.0f;
	bool bAnimating_ = false;

	struct FFlagAnimState
	{
		TObjectPtr<UImage> Image = nullptr;
		float Threshold = 0.0f;
		float CurrentOffset = 0.0f;
		bool bRaised = false;
	};

	TArray<FFlagAnimState> FlagStates_;

	void InitFlags();
	void TickFlags( float deltaTime );
	void ApplyFlagTranslation( FFlagAnimState& flag );
};