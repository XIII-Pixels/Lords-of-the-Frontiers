#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "EnemyRowWidget.generated.h"

class UTextBlock;
class UImage;
struct FEnemyUIData;

UCLASS( Abstract )
class LORDS_FRONTIERS_API UEnemyRowWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetupRow( const FEnemyUIData& data, int32 count );

protected:
	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UImage> Image_Icon;
	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Text_Name;
	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UTextBlock> Text_Count;
	UPROPERTY( meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> Text_Description;
};