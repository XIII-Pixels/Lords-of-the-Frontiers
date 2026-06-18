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
        bool success = AttackComponent_->Attack( hitActor );
		if ( success )
		{
			OnAudioEvent_.Broadcast( { AudioTags_.Attack, GetActorLocation() } );
		}
	}
}

TObjectPtr<UBehaviorTree> ADefensiveBuilding::BehaviorTree() const
{
	return BuildingBehaviorTree_;
}

void ADefensiveBuilding::BeginPlay()
{
	Super::BeginPlay();
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
	if ( !RangeDecalComponent_ )
	{
		return;
	}

	// The radius is silent when it is misconfigured per-Blueprint (e.g. Mage tower / Mortar with no
	// indicator material or a zero range). Log it so the cause is visible instead of "nothing happens".
	if ( !RangeIndicatorMaterial_ )
	{
		UE_LOG(
		    LogTemp, Warning,
		    TEXT( "ADefensiveBuilding::ShowAttackRange: '%s' has no RangeIndicatorMaterial_ set — assign it on "
		          "the building Blueprint, otherwise the attack radius is not drawn." ),
		    *GetName()
		);
		return;
	}

	const float range = Stats().AttackRange();
	if ( range <= 0.0f )
	{
		UE_LOG(
		    LogTemp, Warning,
		    TEXT( "ADefensiveBuilding::ShowAttackRange: '%s' has AttackRange %.1f — the radius decal would be "
		          "invisible. Check the building stats." ),
		    *GetName(), range
		);
	}

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
