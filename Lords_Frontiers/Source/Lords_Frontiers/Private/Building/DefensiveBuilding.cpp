// DefensiveBuilding.cpp

#include "Building/DefensiveBuilding.h"

#include "AI/EntityAIController.h"

#include "Components/Attack/AttackComponent.h"

ADefensiveBuilding::ADefensiveBuilding()
{
	PrimaryActorTick.bCanEverTick = false;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	BuildingAIControllerClass_ = AEntityAIController::StaticClass();
}

void ADefensiveBuilding::OnConstruction( const FTransform& transform )
{
	Super::OnConstruction( transform );
	AIControllerClass = BuildingAIControllerClass_;
}

void ADefensiveBuilding::Attack( TObjectPtr<AActor> hitActor )
{
	if ( AttackComponent_ && !IsDestroyed() )
	{
		AttackComponent_->Attack( hitActor );
	}
}

TObjectPtr<UBehaviorTree> ADefensiveBuilding::BehaviorTree() const
{
	return BuildingBehaviorTree_;
}

TObjectPtr<AActor> ADefensiveBuilding::EnemyInSight() const
{
	if ( AttackComponent_ )
	{
		return AttackComponent_->EnemyInSight();
	}
	return nullptr;
}

void ADefensiveBuilding::BeginPlay()
{
	Super::BeginPlay();

	TArray<UAttackComponent*> attackComponents;
	GetComponents( attackComponents );

	if ( attackComponents.Num() == 1 )
	{
		AttackComponent_ = attackComponents[0];
	}
	else
	{
		UE_LOG(
		    LogTemp, Error, TEXT( "Number of unit attack component is not equal to 1 (number: %d)" ),
		    attackComponents.Num()
		);
	}
}

void ADefensiveBuilding::OnDeath()
{
	Super::OnDeath();

	if ( AttackComponent_ )
	{
		AttackComponent_->DeactivateSight();
	}
}

void ADefensiveBuilding::RestoreFromRuins()
{
	Super::RestoreFromRuins();

	if ( AttackComponent_ )
	{
		AttackComponent_->ActivateSight();
	}
}

void ADefensiveBuilding::FullRestore()
{
	Super::FullRestore();

	if ( AttackComponent_ )
	{
		AttackComponent_->ActivateSight();
	}
}