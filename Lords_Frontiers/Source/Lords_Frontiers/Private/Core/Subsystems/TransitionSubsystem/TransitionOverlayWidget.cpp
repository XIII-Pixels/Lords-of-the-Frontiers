// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Subsystems/TransitionSubsystem/TransitionOverlayWidget.h"

#include "NiagaraSystemWidget.h"

void UTransitionOverlayWidget::PlayAppear( UNiagaraSystem* System )
{
	PlaySystem( System );
}

void UTransitionOverlayWidget::PlayWait( UNiagaraSystem* System )
{
	PlaySystem( System );
}

void UTransitionOverlayWidget::PlayDisappear( UNiagaraSystem* System )
{
	PlaySystem( System );
}

void UTransitionOverlayWidget::PlaySystem( UNiagaraSystem* System )
{
	if ( !TransitionFX || !System )
	{
		return;
	}

	// Keep animating even if the game is paused, swap to the stage's system and restart it.
	TransitionFX->UpdateTickWhenPaused( true );
	TransitionFX->UpdateNiagaraSystemReference( System );
	TransitionFX->ActivateSystem( /*Reset*/ true );
}
