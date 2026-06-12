#pragma once

#include "Resources/GameResource.h"

#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "CoreMinimal.h"

#include "HUDResourcePanelWidget.generated.h"

UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UHUDResourcePanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Text_Citizens;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Text_Gold;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Text_Food;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Text_GoldIncome;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Text_FoodIncome;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UImage> Arrow_Gold;

	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UImage> Arrow_Food;

	UPROPERTY( EditAnywhere, Category = "Settings|UI|Income" )
	float IncomeAnimationDuration = 1.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|UI|Income" )
	float ArrowDisplayDuration = 2.0f;

	UPROPERTY( EditAnywhere, Category = "Settings|UI|Income" )
	FSlateColor PositiveIncomeColor = FSlateColor( FLinearColor( 0.0f, 0.8f, 0.0f, 1.0f ) );

	UPROPERTY( EditAnywhere, Category = "Settings|UI|Income" )
	FSlateColor NegativeIncomeColor = FSlateColor( FLinearColor( 0.9f, 0.1f, 0.1f, 1.0f ) );

	UPROPERTY( EditAnywhere, Category = "Settings|UI|Income" )
	TObjectPtr<UTexture2D> ArrowUpTexture;

	UPROPERTY( EditAnywhere, Category = "Settings|UI|Income" )
	TObjectPtr<UTexture2D> ArrowDownTexture;

	UPROPERTY( EditAnywhere, Category = "Settings|UI|Resources" )
	float ResourceAnimationDuration = 0.4f;

	void UpdateResources();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick( const FGeometry& MyGeometry, float InDeltaTime ) override;

private:
	struct FIncomeAnimState
	{
		int32 StartValue = 0;
		int32 TargetValue = 0;
		int32 DisplayedValue = 0;
		float Elapsed = 0.0f;
		bool bAnimating = false;
		float ArrowTimer = 0.0f;
	};

	FIncomeAnimState GoldIncomeAnim_;
	FIncomeAnimState FoodIncomeAnim_;

	struct FResourceAnimState
	{
		int32 StartValue = 0;
		int32 TargetValue = 0;
		int32 DisplayedValue = 0;
		float Elapsed = 0.0f;
		bool bAnimating = false;
	};

	FResourceAnimState GoldAnim_;
	FResourceAnimState FoodAnim_;
	FResourceAnimState CitizensAnim_;

	bool bIsEconomySubscribed_ = false;

	UFUNCTION()
	void HandleResourceChanged( EResourceType Type, int32 NewAmount );

	UFUNCTION()
	void HandleNetIncomeChanged( const FResourceProduction& netIncome );

	void InitIncomeDisplay();

	void StartIncomeAnimation( UTextBlock* textBlock, UImage* arrow, FIncomeAnimState& state, int32 newValue );

	void TickIncomeAnimation( UTextBlock* textBlock, UImage* arrow, FIncomeAnimState& state, float deltaTime );

	void ApplyIncomeText( UTextBlock* textBlock, int32 value );

	void SetResourceTarget( FResourceAnimState& state, UTextBlock* textBlock, int32 newValue );

	void TickResourceAnimation( FResourceAnimState& state, UTextBlock* textBlock, float deltaTime );

	bool TrySubscribeEconomy();

	void UnsubscribeEconomy();
};
