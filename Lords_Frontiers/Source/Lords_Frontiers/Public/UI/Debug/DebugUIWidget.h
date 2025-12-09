// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "DebugUIWidget.generated.h"


/** (Gregory-hub)
 * UI class for debugging
 * Do not use this for creating actual game UI */
UCLASS(Abstract, Blueprintable)
class LORDS_FRONTIERS_API UDebugUIWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<UButton> Button1;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<UButton> Button2;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<UButton> Button3;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<UButton> Button4;

protected:
	UFUNCTION()
	void OnButton1Clicked();

	UFUNCTION()
	void OnButton2Clicked();

	UFUNCTION()
	void OnButton3Clicked();

	UFUNCTION()
	void OnButton4Clicked();

	virtual bool Initialize() override;
};
