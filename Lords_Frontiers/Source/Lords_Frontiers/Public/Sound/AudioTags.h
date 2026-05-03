// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"

#include "CoreMinimal.h"

/** (Gregory-hub) */
namespace AudioTags
{
	// ── SFX ───────────────────────────────────────────────────────────────────
	inline const FGameplayTag SFX_UI_BUTTON_END_TURN_CLICKED =
	    FGameplayTag::RequestGameplayTag( "SFX.UI.ButtonEndTurn.Clicked" );

	inline const FGameplayTag SFX_BUILDING_SELECTED =
	    FGameplayTag::RequestGameplayTag( "SFX.Building.Selected" );

	// inline const FGameplayTag SFX_Weapon_Fire = FGameplayTag::RequestGameplayTag( "SFX.Weapon.Fire" );
	// inline const FGameplayTag SFX_Weapon_Reload = FGameplayTag::RequestGameplayTag( "SFX.Weapon.Reload" );
	// inline const FGameplayTag SFX_UI_Click = FGameplayTag::RequestGameplayTag( "SFX.UI.Click" );
	// inline const FGameplayTag SFX_Footstep = FGameplayTag::RequestGameplayTag( "SFX.Footstep" );
	//
	// // ── Music ─────────────────────────────────────────────────────────────────
	// inline const FGameplayTag Music = FGameplayTag::RequestGameplayTag( "Music" );
	// inline const FGameplayTag Music_Combat = FGameplayTag::RequestGameplayTag( "Music.Combat" );
	// inline const FGameplayTag Music_Explore = FGameplayTag::RequestGameplayTag( "Music.Explore" );
	// inline const FGameplayTag Music_Forest = FGameplayTag::RequestGameplayTag( "Music.Forest" );
}; // namespace AudioTags
