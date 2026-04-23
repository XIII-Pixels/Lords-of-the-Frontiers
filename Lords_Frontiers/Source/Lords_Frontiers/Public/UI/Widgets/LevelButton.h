// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "LevelButton.generated.h"

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnLevelClicked, int32, LevelIndex );

/** (Gregory-hub)
 * Represents button in level choosing menu */
UCLASS( Abstract )
class LORDS_FRONTIERS_API ULevelButton : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleClick();

	UPROPERTY( BlueprintAssignable )
	FOnLevelClicked OnClicked;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UButton> Butt;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = ( ExposeOnSpawn = true ) )
	int LevelIndex = -1;
};
