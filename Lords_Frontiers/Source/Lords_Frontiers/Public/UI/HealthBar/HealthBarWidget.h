#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"

#include "HealthBarWidget.generated.h"

class UProgressBar;

UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, Category = "HealthBar" )
	void SetHealth( int newHealth, int maxHealth );

	UFUNCTION( BlueprintCallable, Category = "HealthBar" )
	void SnapToTarget();

	UFUNCTION( BlueprintNativeEvent, Category = "HealthBar" )
	void OnShow();
	virtual void OnShow_Implementation()
	{
	}

	UFUNCTION( BlueprintNativeEvent, Category = "HealthBar" )
	void OnHide();
	virtual void OnHide_Implementation()
	{
	}

	void TickAnim( float deltaTime );

	void ApplyCameraScale( float zoomAlpha );

	float MinScale() const
	{
		return MinScale_;
	}

	float MaxScale() const
	{
		return MaxScale_;
	}

protected:
	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|HealthBar|Anim" )
	float LerpSpeed_ = 8.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|HealthBar|Anim" )
	FRuntimeFloatCurve DrainCurve_;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|HealthBar|Scale" )
	float MinScale_ = 0.5f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|HealthBar|Scale" )
	float MaxScale_ = 1.5f;

	float DisplayedPercent_ = 1.0f;

	float DrainStartPercent_ = 1.0f;

	float TargetPercent_ = 1.0f;

	float DrainElapsed_ = 0.0f;
};
