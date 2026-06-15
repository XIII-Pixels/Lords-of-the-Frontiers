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
		PlayAnimation( start_screen );
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
		PlayAnimation( idle_screen );
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
		PlayAnimation( idle_screen );
	}
}

void UTransitionOverlayWidget::StopIdleAfterCycle()
{
	bStopIdleRequested_ = true;

	// If the idle loop is not actually running, finish right away.
	if ( !idle_screen || !IsAnimationPlaying( idle_screen ) )
	{
		OnIdleFinished.Broadcast();
	}
}

void UTransitionOverlayWidget::PlayEnd()
{
	if ( end_screen )
	{
		PlayAnimation( end_screen );
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
