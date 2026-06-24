// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Subsystems/TransitionSubsystem/TransitionOverlayWidget.h"

#include "Animation/WidgetAnimation.h"

void UTransitionOverlayWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if ( start_screen )
	{
		FWidgetAnimationDynamicEvent finished;
		finished.BindDynamic( this, &UTransitionOverlayWidget::HandleStartAnimFinished );
		BindToAnimationFinished( start_screen, finished );
	}
	if ( idle_screen )
	{
		FWidgetAnimationDynamicEvent finished;
		finished.BindDynamic( this, &UTransitionOverlayWidget::HandleIdleAnimFinished );
		BindToAnimationFinished( idle_screen, finished );
	}
	if ( end_screen )
	{
		FWidgetAnimationDynamicEvent finished;
		finished.BindDynamic( this, &UTransitionOverlayWidget::HandleEndAnimFinished );
		BindToAnimationFinished( end_screen, finished );
	}
}

void UTransitionOverlayWidget::PlayStart()
{
	bStopIdleRequested_ = false;
	if ( start_screen )
	{
		// Cover the screen by playing start_screen in reverse (last keyframe -> first), so the
		// animation can be authored in the opposite direction and reused here.
		// BindToAnimationFinished still fires HandleStartAnimFinished when reverse playback completes.
		PlayAnimationReverse( start_screen, AnimationPlaybackSpeed );
	}
	else
	{
		OnStartFinished.Broadcast();
	}
}

void UTransitionOverlayWidget::HandleStartAnimFinished()
{
	OnStartFinished.Broadcast();
}

void UTransitionOverlayWidget::PlayIdleLoop()
{
	bStopIdleRequested_ = false;
	if ( idle_screen )
	{
		PlayAnimation( idle_screen, 0.0f, 1, EUMGSequencePlayMode::Forward, AnimationPlaybackSpeed );
	}
	else
	{
		// No idle animation: nothing to hold on, treat as immediately finished.
		OnIdleFinished.Broadcast();
	}
}

void UTransitionOverlayWidget::HandleIdleAnimFinished()
{
	if ( bStopIdleRequested_ )
	{
		OnIdleFinished.Broadcast();
	}
	else if ( idle_screen )
	{
		// Loading not done yet: keep looping.
		PlayAnimation( idle_screen, 0.0f, 1, EUMGSequencePlayMode::Forward, AnimationPlaybackSpeed );
	}
}

void UTransitionOverlayWidget::StopIdleAtNearestEdge()
{
	bStopIdleRequested_ = true;

	// If the idle loop is not actually running, finish right away.
	if ( !idle_screen || !IsAnimationPlaying( idle_screen ) )
	{
		OnIdleFinished.Broadcast();
		return;
	}

	// Loading is done: settle idle_screen onto whichever edge (start or end) is nearer to the
	// current position. idle_screen is a seamless loop, so both edges share the same "covered"
	// pose from which end_screen takes over. When the settle finishes, HandleIdleAnimFinished
	// broadcasts OnIdleFinished, which kicks off end_screen.
	const float startTime = idle_screen->GetStartTime();
	const float endTime = idle_screen->GetEndTime();
	const float currentTime = GetAnimationCurrentTime( idle_screen );

	if ( ( currentTime - startTime ) <= ( endTime - currentTime ) )
	{
		// Start edge is nearer: rewind from the current position back to the start.
		if ( ( currentTime - startTime ) <= KINDA_SMALL_NUMBER )
		{
			StopAnimation( idle_screen );
			OnIdleFinished.Broadcast();
		}
		else
		{
			PlayAnimationTimeRange( idle_screen, startTime, currentTime, 1, EUMGSequencePlayMode::Reverse, AnimationPlaybackSpeed );
		}
	}
	// else: the end edge is nearer and the loop is already playing forward toward it, so just let
	// this cycle reach the end; bStopIdleRequested_ stops it from looping again.
}

void UTransitionOverlayWidget::PlayEnd()
{
	if ( end_screen )
	{
		PlayAnimation( end_screen, 0.0f, 1, EUMGSequencePlayMode::Forward, AnimationPlaybackSpeed );
	}
	else
	{
		OnEndFinished.Broadcast();
	}
}

void UTransitionOverlayWidget::HandleEndAnimFinished()
{
	OnEndFinished.Broadcast();
}
