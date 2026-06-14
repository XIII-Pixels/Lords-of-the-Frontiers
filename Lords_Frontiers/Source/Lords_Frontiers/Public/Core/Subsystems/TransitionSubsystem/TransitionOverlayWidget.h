// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "TransitionOverlayWidget.generated.h"

class UNiagaraSystem;
class UNiagaraSystemWidget;

/**
 * UMG overlay for the level transition. Added to the viewport at a high Z-order so it
 * draws on top of everything, including the HUD, and keeps playing regardless of world
 * pause or time dilation.
 *
 * The stages are driven entirely from C++ — no Blueprint graph is required. In the widget
 * Blueprint just place a Niagara UI Renderer widget (Niagara System Widget), name it
 * exactly "TransitionFX", stretch it full-screen and set up its UI materials. C++ swaps
 * the system and activates it for each stage.
 *
 * Requires the "Niagara UI Renderer" plugin (provides UNiagaraSystemWidget).
 */
UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API UTransitionOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Play the "appear" stage (cover the screen). */
	void PlayAppear( UNiagaraSystem* System );

	/** Play the "wait" stage (hold while the screen is covered). */
	void PlayWait( UNiagaraSystem* System );

	/** Play the "disappear" stage (reveal the new level). */
	void PlayDisappear( UNiagaraSystem* System );

protected:
	/** Niagara UI Renderer widget that renders the stage particles. Must exist in the
	 *  widget Blueprint with the exact name "TransitionFX". */
	UPROPERTY( meta = ( BindWidget ) )
	TObjectPtr<UNiagaraSystemWidget> TransitionFX;

private:
	/** Swaps the bound Niagara UI widget to the given system and (re)activates it. */
	void PlaySystem( UNiagaraSystem* System );
};
