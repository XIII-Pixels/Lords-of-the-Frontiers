// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "TransitionOverlayWidget.generated.h"

class UWidgetAnimation;

/**
 * UMG loading-screen overlay for level transitions. Added to the viewport at a high
 * Z-order so it draws on top of everything (including the HUD). Driven entirely by C++
 * from three widget animations that must exist in the widget Blueprint with these exact
 * names:
 *   - start_screen : opening animation that covers the screen (played in reverse, so it can be
 *                    authored in the opposite direction and reused).
 *   - idle_screen  : looping hold played while loading finishes.
 *   - end_screen   : closing animation that reveals the new level.
 *
 * Sequence (orchestrated by UTransitionSubsystem across the level switch):
 *   PlayStart -> (OnStartFinished) -> [OpenLevel] -> PlayIdleLoop / StopIdleAtNearestEdge while a
 *   load is still pending (skipped when the map is already loaded) -> PlayEnd -> (OnEndFinished).
 */
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UTransitionOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Plays the opening (cover) animation in reverse. Broadcasts OnStartFinished when done. */
	void PlayStart();

	/** Starts looping the idle animation. */
	void PlayIdleLoop();

	/** Requests the idle loop to stop: idle_screen settles onto whichever edge (start or end) is
	 *  nearer to the current position, then OnIdleFinished fires. */
	void StopIdleAtNearestEdge();

	/** Plays the closing (reveal) animation. Broadcasts OnEndFinished when done. */
	void PlayEnd();

	/** Speed multiplier applied to every transition animation (start / idle / end). 1 = authored
	 *  speed, >1 = faster. Exposed on the WBP so the whole transition can be sped up without
	 *  re-timing keyframes. */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Transition", meta = ( ClampMin = "0.01", UIMin = "0.1", UIMax = "5.0" ) )
	float AnimationPlaybackSpeed = 1.0f;

	/** Fired when start_screen finishes. */
	FSimpleMulticastDelegate OnStartFinished;

	/** Fired when the idle loop has been stopped and its current cycle completed. */
	FSimpleMulticastDelegate OnIdleFinished;

	/** Fired when end_screen finishes. */
	FSimpleMulticastDelegate OnEndFinished;

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY( Transient, meta = ( BindWidgetAnim ) )
	TObjectPtr<UWidgetAnimation> start_screen;

	UPROPERTY( Transient, meta = ( BindWidgetAnim ) )
	TObjectPtr<UWidgetAnimation> idle_screen;

	UPROPERTY( Transient, meta = ( BindWidgetAnim ) )
	TObjectPtr<UWidgetAnimation> end_screen;

	UFUNCTION()
	void HandleStartAnimFinished();

	UFUNCTION()
	void HandleIdleAnimFinished();

	UFUNCTION()
	void HandleEndAnimFinished();

private:
	bool bStopIdleRequested_ = false;
};
