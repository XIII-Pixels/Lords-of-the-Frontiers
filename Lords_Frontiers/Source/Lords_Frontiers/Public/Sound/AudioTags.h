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

    // ── Building ──────────────────────────────────────────────────────────────
    inline const FGameplayTag SFX_BUILDING_SELECTED =
        FGameplayTag::RequestGameplayTag( "SFX.Building.Selected" );
    inline const FGameplayTag SFX_BUILDING_PLACED_SUCCESS =
        FGameplayTag::RequestGameplayTag( "SFX.Building.Placed.Success" );
    inline const FGameplayTag SFX_BUILDING_PLACED_RESTRICTED =
        FGameplayTag::RequestGameplayTag( "SFX.Building.Placed.Restricted" );
    inline const FGameplayTag SFX_BUILDING_DEMOLISHED =
        FGameplayTag::RequestGameplayTag( "SFX.Building.Demolished" );
    inline const FGameplayTag SFX_BUILDING_DEATH =
        FGameplayTag::RequestGameplayTag( "SFX.Building.Death" );
    inline const FGameplayTag SFX_BUILDING_RESURRECTION =
        FGameplayTag::RequestGameplayTag( "SFX.Building.Resurrection" );

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

    // ── Unit ── Wolf-goblin ───────────────────────────────────────────────────
    inline const FGameplayTag SFX_UNIT_WOLFGOBLIN_SELECTED =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Wolf-goblin.Selected" );
    inline const FGameplayTag SFX_UNIT_WOLFGOBLIN_SPAWN =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Wolf-goblin.Spawn" );
    inline const FGameplayTag SFX_UNIT_WOLFGOBLIN_DEATH =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Wolf-goblin.Death" );
    inline const FGameplayTag SFX_UNIT_WOLFGOBLIN_ATTACK =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Wolf-goblin.Attack" );
    inline const FGameplayTag SFX_UNIT_WOLFGOBLIN_TAKEDAMAGE =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Wolf-goblin.TakeDamage" );
    inline const FGameplayTag SFX_UNIT_WOLFGOBLIN_SPAWNUNIT =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Wolf-goblin.SpawnUnit" );

    // ── Unit ── Chihua-goblin ─────────────────────────────────────────────────
    inline const FGameplayTag SFX_UNIT_CHIHUAGOBLIN_SELECTED =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Chihua-goblin.Selected" );
    inline const FGameplayTag SFX_UNIT_CHIHUAGOBLIN_SPAWN =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Chihua-goblin.Spawn" );
    inline const FGameplayTag SFX_UNIT_CHIHUAGOBLIN_DEATH =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Chihua-goblin.Death" );
    inline const FGameplayTag SFX_UNIT_CHIHUAGOBLIN_ATTACK =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Chihua-goblin.Attack" );
    inline const FGameplayTag SFX_UNIT_CHIHUAGOBLIN_TAKEDAMAGE =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Chihua-goblin.TakeDamage" );
    inline const FGameplayTag SFX_UNIT_CHIHUAGOBLIN_SPAWNUNIT =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Chihua-goblin.SpawnUnit" );

    // ── Unit ── Dog-ogr ───────────────────────────────────────────────────────
    inline const FGameplayTag SFX_UNIT_DOGOGR_SELECTED =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Dog-ogr.Selected" );
    inline const FGameplayTag SFX_UNIT_DOGOGR_SPAWN =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Dog-ogr.Spawn" );
    inline const FGameplayTag SFX_UNIT_DOGOGR_DEATH =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Dog-ogr.Death" );
    inline const FGameplayTag SFX_UNIT_DOGOGR_ATTACK =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Dog-ogr.Attack" );
    inline const FGameplayTag SFX_UNIT_DOGOGR_TAKEDAMAGE =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Dog-ogr.TakeDamage" );
    inline const FGameplayTag SFX_UNIT_DOGOGR_SPAWNUNIT =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Dog-ogr.SpawnUnit" );

    // ── Unit ── Don ───────────────────────────────────────────────────────────
    inline const FGameplayTag SFX_UNIT_DON_SELECTED =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Don.Selected" );
    inline const FGameplayTag SFX_UNIT_DON_SPAWN =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Don.Spawn" );
    inline const FGameplayTag SFX_UNIT_DON_DEATH =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Don.Death" );
    inline const FGameplayTag SFX_UNIT_DON_ATTACK =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Don.Attack" );
    inline const FGameplayTag SFX_UNIT_DON_TAKEDAMAGE =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Don.TakeDamage" );
    inline const FGameplayTag SFX_UNIT_DON_SPAWNUNIT =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Don.SpawnUnit" );

    // ── Unit ── Wizard ────────────────────────────────────────────────────────
    inline const FGameplayTag SFX_UNIT_WIZARD_SELECTED =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Wizard.Selected" );
    inline const FGameplayTag SFX_UNIT_WIZARD_SPAWN =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Wizard.Spawn" );
    inline const FGameplayTag SFX_UNIT_WIZARD_DEATH =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Wizard.Death" );
    inline const FGameplayTag SFX_UNIT_WIZARD_ATTACK =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Wizard.Attack" );
    inline const FGameplayTag SFX_UNIT_WIZARD_TAKEDAMAGE =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Wizard.TakeDamage" );
    inline const FGameplayTag SFX_UNIT_WIZARD_SPAWNUNIT =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Wizard.SpawnUnit" );

    // ── Unit ── Flying ────────────────────────────────────────────────────────
    inline const FGameplayTag SFX_UNIT_FLYING_SELECTED =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Flying.Selected" );
    inline const FGameplayTag SFX_UNIT_FLYING_SPAWN =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Flying.Spawn" );
    inline const FGameplayTag SFX_UNIT_FLYING_DEATH =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Flying.Death" );
    inline const FGameplayTag SFX_UNIT_FLYING_ATTACK =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Flying.Attack" );
    inline const FGameplayTag SFX_UNIT_FLYING_TAKEDAMAGE =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Flying.TakeDamage" );
    inline const FGameplayTag SFX_UNIT_FLYING_SPAWNUNIT =
        FGameplayTag::RequestGameplayTag( "SFX.Unit.Flying.SpawnUnit" );

}; // namespace AudioTags
