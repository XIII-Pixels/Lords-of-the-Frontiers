// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "WinLoseWidget.generated.h"

class UButton;

/** (Gregory-hub)
 * Base class for win widget and lose widget */
UCLASS( Abstract )
class LORDS_FRONTIERS_API UWinLoseWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UButton> MainMenuButton;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UButton> NewGameButton;

	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UButton> QuitButton;
};
