#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "CoreMinimal.h"

#include "ResourceItemWidget.generated.h"

UCLASS( Abstract )
class LORDS_FRONTIERS_API UResourceItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetupItem( UTexture2D* iconTexture, int32 initialValue, int32 maxValue );

	void UpdateAmount( int32 newAmount, int32 maxAmount );

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UImage> ResourceIcon;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UTextBlock> ResourceAmountText;

private:
	int32 CurrentValue_ = 0;
};
