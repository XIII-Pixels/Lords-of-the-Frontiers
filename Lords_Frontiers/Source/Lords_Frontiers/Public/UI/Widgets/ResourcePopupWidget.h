// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "ResourcePopupWidget.generated.h"

class UImage;
class UTextBlock;

/**
 *
 */
UCLASS()
class LORDS_FRONTIERS_API UResourcePopupWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, Category = "Settings|ResourcePopup" )
	void SetData( UTexture2D* icon, int32 amount, bool bIsBonus, bool bIsRuined );

	void SetPopupOpacity( float alpha );

	void SetIconOnly( bool bIconOnly );

protected:
	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UImage> ResourceIcon;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UTextBlock> AmountText;
};
