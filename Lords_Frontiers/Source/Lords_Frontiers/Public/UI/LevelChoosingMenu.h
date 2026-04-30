// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "LevelChoosingMenu.generated.h"

class UButton;

/** (Gregory-hub) */
UCLASS( Abstract )
class LORDS_FRONTIERS_API ULevelChoosingMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UButton> BackButton;
};
