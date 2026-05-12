// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/GameLoop/GameLoopRewardHelper.h"

#include "Core/GameLoop/GameLoopConfig.h"
#include "Resources/EconomyComponent.h"
#include "Resources/ResourceManager.h"

void UGameLoopRewardHelper::Initialize( UGameLoopConfig* config, UResourceManager* rm, UEconomyComponent* ec )
{
	Config_ = config;
	ResourceManager_ = rm;
	EconomyComponent_ = ec;
}

void UGameLoopRewardHelper::GrantStartingResources()
{
	if ( !Config_ || !ResourceManager_.IsValid() )
	{
		return;
	}

	FResourceReward& reward = Config_->StartingResources;
	ApplyReward( reward );
}

void UGameLoopRewardHelper::GrantCombatReward( int32 wave, bool bPerfectWave )
{
	if ( !Config_ || !ResourceManager_.IsValid() )
	{
		return;
	}

	FResourceReward reward = Config_->GetScaledCombatReward( wave, bPerfectWave );
	ApplyReward( reward );
}

void UGameLoopRewardHelper::ApplyReward( FResourceReward& reward )
{
	if ( reward.Gold > 0 )
	{
		ResourceManager_->AddResource( EResourceType::Gold, reward.Gold );
	}
	if ( reward.Food > 0 )
	{
		ResourceManager_->AddResource( EResourceType::Food, reward.Food );
	}
	if ( reward.Population > 0 )
	{
		ResourceManager_->AddResource( EResourceType::Population, reward.Population );
	}
}

void UGameLoopRewardHelper::CollectBuildingIncome()
{
	if ( UEconomyComponent* ec = EconomyComponent_.Get() )
	{
		ec->CollectGlobalResources();
	}
}

void UGameLoopRewardHelper::ApplyMaintenanceCosts()
{
	if ( UEconomyComponent* ec = EconomyComponent_.Get() )
	{
		ec->ApplyMaintenanceCosts();
	}
}

void UGameLoopRewardHelper::RecalculateIncome()
{
	if ( UEconomyComponent* ec = EconomyComponent_.Get() )
	{
		ec->PerformInitialScan();
		ec->RecalculateAndBroadcastNetIncome();
	}
}

void UGameLoopRewardHelper::RestoreBuildings()
{
	if ( UEconomyComponent* ec = EconomyComponent_.Get() )
	{
		ec->RestoreAllBuildings();
	}
}


