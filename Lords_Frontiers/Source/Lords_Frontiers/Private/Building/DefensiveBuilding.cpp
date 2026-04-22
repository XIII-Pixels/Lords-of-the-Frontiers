// DefensiveBuilding.cpp

#include "Building/DefensiveBuilding.h"

#include "AI/EntityAIController.h"

#include "Components/Attack/BuildingAttackRangedComponent.h"
#include "Components/DecalComponent.h"

ADefensiveBuilding::ADefensiveBuilding()
{
	PrimaryActorTick.bCanEverTick = false;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	BuildingAIControllerClass_ = AEntityAIController::StaticClass();

	AttackComponent_ = CreateDefaultSubobject<UBuildingAttackRangedComponent>( TEXT( "Attack Ranged " ) );
	AttackComponent_->SetupAttachment( RootComponent );

	RangeDecalComponent_ = CreateDefaultSubobject<UDecalComponent>( TEXT( "RangeDecal" ) );
	RangeDecalComponent_->SetupAttachment( RootComponent );
	RangeDecalComponent_->SetRelativeRotation( FRotator( -90.f, 0.f, 0.f ) );
	RangeDecalComponent_->SetVisibility( false );
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

void ADefensiveBuilding::BeginPlay()
{
	Super::BeginPlay();

	// TArray<UBuildingAttackRangedComponent*> attackComponents;
	// GetComponents( attackComponents );
	//
	// if ( attackComponents.Num() == 1 )
	// {
	// 	AttackComponent_ = attackComponents[0];
	// }
	// else
	// {
	// 	UE_LOG(
	// 	    LogTemp, Error, TEXT( "Number of unit attack component is not equal to 1 (number: %d)" ),
	// 	    attackComponents.Num()
	// 	);
	// }
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

void ADefensiveBuilding::ShowAttackRange()
{
	if ( !RangeDecalComponent_ || !RangeIndicatorMaterial_ )
	{
		return;
	}

	const float range = Stats().AttackRange();
	RangeDecalComponent_->DecalSize = FVector( range, range, range );
	RangeDecalComponent_->SetDecalMaterial( RangeIndicatorMaterial_ );
	RangeDecalComponent_->SetVisibility( true );
}

void ADefensiveBuilding::HideAttackRange()
{
	if ( RangeDecalComponent_ )
	{
		RangeDecalComponent_->SetVisibility( false );
	}
}
