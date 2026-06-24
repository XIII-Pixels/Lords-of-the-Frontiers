// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "LevelChoosingMenu.generated.h"

class UTextBlock;
class UTextButtonWidget;

/** (Gregory-hub) */
UCLASS( Abstract )
class LORDS_FRONTIERS_API ULevelChoosingMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Back button (WBP based on UTextButtonWidget). Caption is localized and glow handled by that widget. */
	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UTextButtonWidget> BackButton;

	/** Screen title. Localized from ST_GameStrings (LevelSelect.Title) in C++ — no WBP picker needed. */
	UPROPERTY( BlueprintReadOnly, Category = "LevelChoosing", meta = ( BindWidgetOptional ) )
	TObjectPtr<UTextBlock> TitleText;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

private:
	void ApplyLocalizedLabels();
};
