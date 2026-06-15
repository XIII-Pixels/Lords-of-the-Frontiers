// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Templates/SubclassOf.h"

#include "TransitionConfig.generated.h"

class UTransitionOverlayWidget;

/**
 * Configuration for the level-transition loading screen.
 *
 * The transition is a UMG widget with three widget animations played across the level
 * switch (OpenLevel destroys the old viewport widget, so the stages run on two levels):
 *   1. start_screen - plays on the OLD level to cover the screen.
 *   2. idle_screen  - loops on the NEW level while loading finishes; once loading is done
 *                     the current loop is allowed to complete before revealing.
 *   3. end_screen   - plays on the NEW level to reveal the gameplay.
 */
UCLASS()
class LORDS_FRONTIERS_API UTransitionConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Widget (with start_screen / idle_screen / end_screen animations) shown during the transition. */
	UPROPERTY( EditDefaultsOnly, Category = "Settings|Transition" )
	TSubclassOf<UTransitionOverlayWidget> OverlayWidgetClass;

	/** Viewport Z-order for the overlay (higher draws on top; the HUD is usually 0). */
	UPROPERTY( EditDefaultsOnly, Category = "Settings|Transition" )
	int32 OverlayZOrder = 1000;

	/** Unpause the game when a transition starts so the animations and load can run. */
	UPROPERTY( EditDefaultsOnly, Category = "Settings|Transition" )
	bool bUnpauseOnTransition = true;
};
