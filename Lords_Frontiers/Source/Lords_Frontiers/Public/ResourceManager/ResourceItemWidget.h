#pragma once

#include "Lords_Frontiers/Public/ResourceManager/GameResource.h"

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
	void SetupItem( UTexture2D* IconTexture, int32 InitialValue, int32 MaxValue );

	void UpdateAmount( int32 NewAmount, int32 MaxAmount );

protected:
	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UImage> ResourceIcon;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UTextBlock> ResourceAmountText;

private:
	int32 CurrentValue_ = 0;
};