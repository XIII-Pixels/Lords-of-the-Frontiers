#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "Sound/AudioEvent.h"
#include "Sound/AudioEventSource.h"

#include "WaveInfoPanelWidget.generated.h"

class AUnit;
class UVerticalBox;
class UEnemyInfoDataAsset;
class UEnemyRowWidget;
class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnWavePanelStateChanged, bool, bIsOpen );

UCLASS( Abstract )
class LORDS_FRONTIERS_API UWaveInfoPanelWidget : public UUserWidget, public IAudioEventSource
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick( const FGeometry& myGeometry, float inDeltaTime ) override;

	void PopulatePanel( const TMap<TSubclassOf<AUnit>, int32>& waveData );

	UFUNCTION( BlueprintCallable, Category = "UI|WaveInfo" )
	void OpenPanel();

	UFUNCTION( BlueprintCallable, Category = "UI|WaveInfo" )
	void ClosePanel();

	UFUNCTION( BlueprintCallable, Category = "UI|WaveInfo" )
	void TogglePanel();

	UFUNCTION( BlueprintPure, Category = "UI|WaveInfo" )
	bool IsPanelOpen() const
	{
		return bIsOpen_;
	}

	UPROPERTY( BlueprintAssignable, Category = "UI|WaveInfo" )
	FOnWavePanelStateChanged OnPanelStateChanged;

protected:
	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UWidget> SlideContent;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UVerticalBox> EnemyListContainer;

	UPROPERTY( BlueprintReadOnly, meta = ( BindWidget ), Category = "UI|WaveInfo" )
	TObjectPtr<UButton> ButtonToggle;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Text_TotalEnemies;

	UPROPERTY( EditAnywhere, Category = "Settings|Data" )
	TObjectPtr<UEnemyInfoDataAsset> EnemyDataAsset;

	UPROPERTY( EditAnywhere, Category = "Settings|Classes" )
	TSubclassOf<UEnemyRowWidget> EnemyRowClass;

	UPROPERTY( EditAnywhere, Category = "Settings|Layout", meta = ( ClampMin = "1" ) )
	int32 ColumnCount_ = 2;

	UPROPERTY( EditAnywhere, Category = "Settings|Layout" )
	bool bFillBottomUp_ = true;

	UPROPERTY( EditAnywhere, Category = "Settings|Layout", meta = ( EditCondition = "bFillBottomUp_", ClampMin = "0.0" ) )
	float EnemyListBottomPadding_ = 32.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|Animation", meta = ( ClampMin = "0.01" ) )
	float AnimDuration_ = 0.3f;

	UPROPERTY( EditAnywhere, Category = "Settings|Animation", meta = ( ClampMin = "0.0" ) )
	float SlidePadding_ = 16.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|Animation", meta = ( ClampMin = "0.0" ) )
	float MinSlideDistance_ = 60.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|Animation", meta = ( ClampMin = "0.0" ) )
	float FallbackSlideDistance_ = 200.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|Animation|Hover", meta = ( ClampMin = "0.0" ) )
	float HoverPeekDistance_ = 20.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|Animation|Hover", meta = ( ClampMin = "0.01" ) )
	float HoverAnimDuration_ = 0.15f;

	UPROPERTY( EditAnywhere, Category = "Settings|Button" )
	FVector2D ButtonClosedSize_ = FVector2D( 120.0f, 60.0f );

	UPROPERTY( EditAnywhere, Category = "Settings|State" )
	bool bStartOpen_ = false;

	virtual FOnAudioEvent& GetOnAudioEvent() override
	{
		return OnAudioEvent_;
	}

private:
	UFUNCTION()
	void OnButtonToggleClicked();

	UFUNCTION()
	void OnButtonToggleHovered();

	UFUNCTION()
	void OnButtonToggleUnhovered();

	void ApplyCurrentOffset();
	void ApplyButtonSize();
	void RefreshSlideDistance();
	void ApplyBottomAlignment();
	float ComputeBaseTargetY() const;
	float ComputeHoverTargetY() const;

	bool bIsOpen_ = false;
	bool bButtonHovered_ = false;
	float CurrentBaseY_ = 0.0f;
	float CurrentHoverY_ = 0.0f;
	float CurrentY_ = 0.0f;
	float CachedSlideDistance_ = 200.0f;

	UPROPERTY( Transient )
	TMap<TSubclassOf<AUnit>, TObjectPtr<UEnemyRowWidget>> ActiveRowsMap_;

	FOnAudioEvent OnAudioEvent_;
};
