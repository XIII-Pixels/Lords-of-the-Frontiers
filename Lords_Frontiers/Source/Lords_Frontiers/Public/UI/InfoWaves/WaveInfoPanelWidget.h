#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "WaveInfoPanelWidget.generated.h"

class AUnit;
class UPanelWidget;
class UEnemyInfoDataAsset;
class UEnemyRowWidget;
class UCurveFloat;

UCLASS( Abstract )
class LORDS_FRONTIERS_API UWaveInfoPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
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

protected:
	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UWidget> SlideContainer;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UPanelWidget> EnemyListContainer;

	UPROPERTY( EditAnywhere, Category = "Settings|Data" )
	TObjectPtr<UEnemyInfoDataAsset> EnemyDataAsset;

	UPROPERTY( EditAnywhere, Category = "Settings|Classes" )
	TSubclassOf<UEnemyRowWidget> EnemyRowClass;

	UPROPERTY( EditAnywhere, Category = "Settings|Animation" )
	float AnimDuration_ = 0.3f;

	UPROPERTY( EditAnywhere, Category = "Settings|Animation" )
	TObjectPtr<UCurveFloat> SlideCurve_;

	UPROPERTY( EditAnywhere, Category = "Settings|Animation" )
	FVector2D ClosedOffset_ = FVector2D( 0.0f, -300.0f );

	UPROPERTY( EditAnywhere, Category = "Settings|Animation" )
	FVector2D OpenOffset_ = FVector2D( 0.0f, 0.0f );

private:
	bool bIsOpen_ = true;
	float AnimProgress_ = 1.0f;

	UPROPERTY( Transient )
	TMap<TSubclassOf<AUnit>, TObjectPtr<UEnemyRowWidget>> ActiveRowsMap_;
};