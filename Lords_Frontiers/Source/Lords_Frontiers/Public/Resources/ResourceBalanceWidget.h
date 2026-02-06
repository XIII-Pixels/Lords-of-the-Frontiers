#pragma once

#include "Blueprint/UserWidget.h"
#include "Lords_Frontiers/Public/Resources/GameResource.h"

#include "ResourceBalanceWidget.generated.h"

class UTextBlock;
class UEconomyComponent;

/**
 * UResourceBalanceWidget
 *
 * Displays net income per resource type under the main resource bar.
 * Positive values shown in green ("+16"), negative in red ("-141").
 *
 * UMG Setup:
 *   Create a Widget Blueprint that inherits from this class.
 *   Add four TextBlocks named exactly:
 *     Text_Gold_Net, Text_Food_Net, Text_Pop_Net, Text_Progress_Net
 *   Place them under your existing resource icons.
 */
UCLASS()
class LORDS_FRONTIERS_API UResourceBalanceWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	UFUNCTION()
	void OnBalanceChanged( const FResourceProduction& NetIncome );

	/** Format value and set color: green (+), red (-), gray (0). */
	void UpdateDisplay( UTextBlock* TextBlock, int32 Value );

	UPROPERTY( meta = ( BindWidget ) )
	UTextBlock* Text_Gold_Net;

	UPROPERTY( meta = ( BindWidget ) )
	UTextBlock* Text_Food_Net;

	UPROPERTY( meta = ( BindWidget ) )
	UTextBlock* Text_Pop_Net;

	UPROPERTY( meta = ( BindWidget ) )
	UTextBlock* Text_Progress_Net;

private:
	UPROPERTY()
	UEconomyComponent* CachedEconomy_;
};
