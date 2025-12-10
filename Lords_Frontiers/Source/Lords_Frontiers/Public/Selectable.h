// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "Selectable.generated.h"

/**
 *
 */
UINTERFACE( BlueprintType )
class LORDS_FRONTIERS_API USelectable : public UInterface
{
	GENERATED_BODY()
};

class LORDS_FRONTIERS_API ISelectable
{
	GENERATED_BODY()

  public:
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "Setting|Selection" )
	void OnSelected();

	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "Setting|Selection" )
	void OnDeselected();

	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "Setting|Selection" )
	bool CanBeSelected() const;

	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "Setting|Selection" )
	FVector GetSelectionLocation() const;
};