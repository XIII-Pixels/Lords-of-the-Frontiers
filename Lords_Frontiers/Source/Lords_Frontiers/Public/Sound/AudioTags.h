// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"

#include "CoreMinimal.h"

/** (Gregory-hub) */
namespace AudioTags
{
    // ── UI ── MainMenu ────────────────────────────────────────────────────────
    inline const FGameplayTag SFX_UI_MAINMENU_BUTTONS_HOVERED =
        FGameplayTag::RequestGameplayTag( "SFX.UI.MainMenu.Buttons.Hovered" );
    inline const FGameplayTag SFX_UI_MAINMENU_BUTTONS_CLICKED =
        FGameplayTag::RequestGameplayTag( "SFX.UI.MainMenu.Buttons.Clicked" );

    // ── UI ── LevelChoosingMenu ───────────────────────────────────────────────
    inline const FGameplayTag SFX_UI_LEVELCHOOSINGMENU_BUTTONS_HOVERED =
        FGameplayTag::RequestGameplayTag( "SFX.UI.LevelChoosingMenu.Buttons.Hovered" );
    inline const FGameplayTag SFX_UI_LEVELCHOOSINGMENU_BUTTONS_CLICKED =
        FGameplayTag::RequestGameplayTag( "SFX.UI.LevelChoosingMenu.Buttons.Clicked" );

    // ── UI ── Buttons ─────────────────────────────────────────────────────────
    inline const FGameplayTag SFX_UI_BUTTON_ENDTURN_HOVERED =
        FGameplayTag::RequestGameplayTag( "SFX.UI.Button.EndTurn.Hovered" );
    inline const FGameplayTag SFX_UI_BUTTON_ENDTURN_CLICKED =
        FGameplayTag::RequestGameplayTag( "SFX.UI.Button.EndTurn.Clicked" );

    inline const FGameplayTag SFX_UI_BUTTON_BUILDING_HOVERED =
        FGameplayTag::RequestGameplayTag( "SFX.UI.Button.Building.Hovered" );
    inline const FGameplayTag SFX_UI_BUTTON_BUILDING_CLICKED =
        FGameplayTag::RequestGameplayTag( "SFX.UI.Button.Building.Clicked" );

    inline const FGameplayTag SFX_UI_BUTTON_MOVEBUILDING_HOVERED =
        FGameplayTag::RequestGameplayTag( "SFX.UI.Button.MoveBuilding.Hovered" );
    inline const FGameplayTag SFX_UI_BUTTON_MOVEBUILDING_CLICKED =
        FGameplayTag::RequestGameplayTag( "SFX.UI.Button.MoveBuilding.Clicked" );

    inline const FGameplayTag SFX_UI_BUTTON_DEMOLISHBUILDING_HOVERED =
        FGameplayTag::RequestGameplayTag( "SFX.UI.Button.DemolishBuilding.Hovered" );
    inline const FGameplayTag SFX_UI_BUTTON_DEMOLISHBUILDING_CLICKED =
        FGameplayTag::RequestGameplayTag( "SFX.UI.Button.DemolishBuilding.Clicked" );

    inline const FGameplayTag SFX_UI_BUTTON_BUILDINGCATEGORY_HOVERED =
        FGameplayTag::RequestGameplayTag( "SFX.UI.Button.BuildingCategory.Hovered" );
    inline const FGameplayTag SFX_UI_BUTTON_BUILDINGCATEGORY_CLICKED =
        FGameplayTag::RequestGameplayTag( "SFX.UI.Button.BuildingCategory.Clicked" );

    inline const FGameplayTag SFX_UI_BUTTON_NEXTWAVEINFO_HOVERED =
        FGameplayTag::RequestGameplayTag( "SFX.UI.Button.NextWaveInfo.Hovered" );
    inline const FGameplayTag SFX_UI_BUTTON_NEXTWAVEINFO_CLICKED =
        FGameplayTag::RequestGameplayTag( "SFX.UI.Button.NextWaveInfo.Clicked" );

    inline const FGameplayTag SFX_UI_BUTTON_TIME_HOVERED =
        FGameplayTag::RequestGameplayTag( "SFX.UI.Button.Time.Hovered" );
    inline const FGameplayTag SFX_UI_BUTTON_TIME_CLICKED =
        FGameplayTag::RequestGameplayTag( "SFX.UI.Button.Time.Clicked" );

    inline const FGameplayTag SFX_UI_BUTTON_TIME_PAUSE_HOVERED =
        FGameplayTag::RequestGameplayTag( "SFX.UI.Button.Time.Pause.Hovered" );
    inline const FGameplayTag SFX_UI_BUTTON_TIME_PAUSE_CLICKED =
        FGameplayTag::RequestGameplayTag( "SFX.UI.Button.Time.Pause.Clicked" );

    // ── Pause game ────────────────────────────────────────────────────────────
    inline const FGameplayTag SFX_PAUSEGAME =
        FGameplayTag::RequestGameplayTag( "SFX.PauseGame" );
    inline const FGameplayTag SFX_RESUMEGAME =
        FGameplayTag::RequestGameplayTag( "SFX.ResumeGame" );

    // ── Resources ─────────────────────────────────────────────────────────────
    inline const FGameplayTag SFX_RESOURCES_DEFAULT_AWARDED =
        FGameplayTag::RequestGameplayTag( "SFX.Resources.Default.Awarded" );
    inline const FGameplayTag SFX_RESOURCES_GOLD_AWARDED =
        FGameplayTag::RequestGameplayTag( "SFX.Resources.Gold.Awarded" );
    inline const FGameplayTag SFX_RESOURCES_FOOD_AWARDED =
        FGameplayTag::RequestGameplayTag( "SFX.Resources.Food.Awarded" );
    inline const FGameplayTag SFX_RESOURCES_POPULATION_AWARDED =
        FGameplayTag::RequestGameplayTag( "SFX.Resources.Population.Awarded" );

    // ── GamePhase ─────────────────────────────────────────────────────────────
    inline const FGameplayTag SFX_GAMEPHASE_STARTGAME =
        FGameplayTag::RequestGameplayTag( "SFX.GamePhase.StartGame" );
    inline const FGameplayTag SFX_GAMEPHASE_LOSEGAME =
        FGameplayTag::RequestGameplayTag( "SFX.GamePhase.LoseGame" );
    inline const FGameplayTag SFX_GAMEPHASE_WINGAME =
        FGameplayTag::RequestGameplayTag( "SFX.GamePhase.WinGame" );

    inline const FGameplayTag SFX_GAMEPHASE_BUILDING_START =
        FGameplayTag::RequestGameplayTag( "SFX.GamePhase.Building.Start" );

    inline const FGameplayTag SFX_GAMEPHASE_COMBAT_START =
        FGameplayTag::RequestGameplayTag( "SFX.GamePhase.Combat.Start" );
    inline const FGameplayTag SFX_GAMEPHASE_COMBAT_WIN =
        FGameplayTag::RequestGameplayTag( "SFX.GamePhase.Combat.Win" );

    inline const FGameplayTag SFX_GAMEPHASE_REWARD_START =
        FGameplayTag::RequestGameplayTag( "SFX.GamePhase.Reward.Start" );
    inline const FGameplayTag SFX_GAMEPHASE_REWARD_END =
        FGameplayTag::RequestGameplayTag( "SFX.GamePhase.Reward.End" );

    // ── Building - Default ────────────────────────────────────────────────────
    inline const FGameplayTag SFX_BUILDING_DEFAULT_SELECTED =
        FGameplayTag::RequestGameplayTag( "SFX.Building.Default.Selected" );
    inline const FGameplayTag SFX_BUILDING_DEFAULT_PLACED_SUCCESS =
        FGameplayTag::RequestGameplayTag( "SFX.Building.Default.Placed.Success" );
    inline const FGameplayTag SFX_BUILDING_DEFAULT_PLACED_RESTRICTED =
        FGameplayTag::RequestGameplayTag( "SFX.Building.Default.Placed.Restricted" );
    inline const FGameplayTag SFX_BUILDING_DEFAULT_DEMOLISHED =
        FGameplayTag::RequestGameplayTag( "SFX.Building.Default.Demolished" );
    inline const FGameplayTag SFX_BUILDING_DEFAULT_DEATH =
        FGameplayTag::RequestGameplayTag( "SFX.Building.Default.Death" );
    inline const FGameplayTag SFX_BUILDING_DEFAULT_RESURRECTED =
        FGameplayTag::RequestGameplayTag( "SFX.Building.Default.Resurrected" );
    inline const FGameplayTag SFX_BUILDING_DEFAULT_ATTACK =
        FGameplayTag::RequestGameplayTag( "SFX.Building.Default.Attack" );
    inline const FGameplayTag SFX_BUILDING_DEFAULT_TAKEDAMAGE =
        FGameplayTag::RequestGameplayTag( "SFX.Building.Default.TakeDamage" );

    // ── Unit ── Default ───────────────────────────────────────────────────────
    inline const FGameplayTag SFX_UNIT_DEFAULT_SELECTED =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Default.Selected" );
    inline const FGameplayTag SFX_UNIT_DEFAULT_SPAWN =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Default.Spawn" );
    inline const FGameplayTag SFX_UNIT_DEFAULT_DEATH =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Default.Death" );
    inline const FGameplayTag SFX_UNIT_DEFAULT_ATTACK =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Default.Attack" );
    inline const FGameplayTag SFX_UNIT_DEFAULT_TAKEDAMAGE =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Default.TakeDamage" );
    inline const FGameplayTag SFX_UNIT_DEFAULT_SPAWNUNIT =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Default.SpawnUnit" );
}; // namespace AudioTags
