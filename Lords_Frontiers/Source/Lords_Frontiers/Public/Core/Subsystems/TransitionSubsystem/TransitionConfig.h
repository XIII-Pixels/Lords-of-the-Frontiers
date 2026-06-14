// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Templates/SubclassOf.h"

#include "TransitionConfig.generated.h"

class UNiagaraSystem;
class UTransitionOverlayWidget;

/**
 * Configuration for the level-transition Niagara effect.
 *
 * The effect is split into three separate Niagara systems that play across the
 * level switch. OpenLevel destroys the old world, so "appear" and "disappear"
 * physically run on different levels:
 *   1. Appear    - plays on the OLD level until the screen is covered.
 *   2. Wait      - optional looping hold that plays on the NEW level right after
 *                  load, while the screen is still covered.
 *   3. Disappear - plays on the NEW level to reveal the gameplay.
 */
UCLASS()
class LORDS_FRONTIERS_API UTransitionConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Plays on the old level to cover the screen before the new level is opened. */
	UPROPERTY( EditDefaultsOnly, Category = "Settings|Transition|VFX" )
	TObjectPtr<UNiagaraSystem> AppearVFX;

	/** Optional looping hold shown on the new level while still covered. May be null. */
	UPROPERTY( EditDefaultsOnly, Category = "Settings|Transition|VFX" )
	TObjectPtr<UNiagaraSystem> WaitVFX;

	/** Plays on the new level to reveal the gameplay. */
	UPROPERTY( EditDefaultsOnly, Category = "Settings|Transition|VFX" )
	TObjectPtr<UNiagaraSystem> DisappearVFX;

	/**
	 * Optional UMG overlay used to render the transition on top of everything (including
	 * the HUD), via the Niagara UI Renderer plugin. When set, this is used instead of
	 * spawning world-space Niagara, and the three systems above are passed to the widget's
	 * PlayAppear / PlayWait / PlayDisappear events.
	 */
	UPROPERTY( EditDefaultsOnly, Category = "Settings|Transition|Overlay" )
	TSubclassOf<UTransitionOverlayWidget> OverlayWidgetClass;

	/** Viewport Z-order for the overlay (higher draws on top; the HUD is usually 0). */
	UPROPERTY( EditDefaultsOnly, Category = "Settings|Transition|Overlay" )
	int32 OverlayZOrder = 1000;

	/** How long the appear stage plays before the new level is opened (seconds). */
	UPROPERTY( EditDefaultsOnly, Category = "Settings|Transition|Timing", meta = ( ClampMin = "0.0" ) )
	float AppearDuration = 0.6f;

	/** How long the wait stage holds on the new level before revealing (seconds). */
	UPROPERTY( EditDefaultsOnly, Category = "Settings|Transition|Timing", meta = ( ClampMin = "0.0" ) )
	float WaitDuration = 0.2f;

	/** How long the disappear stage plays while revealing the new level (seconds). */
	UPROPERTY( EditDefaultsOnly, Category = "Settings|Transition|Timing", meta = ( ClampMin = "0.0" ) )
	float DisappearDuration = 0.6f;

	/** Offset from the camera in relative space (X = forward) where the effect is placed. */
	UPROPERTY( EditDefaultsOnly, Category = "Settings|Transition|Placement" )
	FVector CameraOffset = FVector( 200.0f, 0.0f, 0.0f );

	/** Per-axis scale applied to the spawned effect. */
	UPROPERTY( EditDefaultsOnly, Category = "Settings|Transition|Placement" )
	FVector Scale = FVector::OneVector;

	/** Unpause the game when a transition starts so the timed stages can run. */
	UPROPERTY( EditDefaultsOnly, Category = "Settings|Transition" )
	bool bUnpauseOnTransition = true;
};
