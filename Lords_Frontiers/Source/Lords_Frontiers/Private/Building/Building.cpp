#include "Lords_Frontiers/Public/Building/Building.h"

ABuilding::ABuilding()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create and set root component
	BuildingMesh_ = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "BuildingMesh" ) );
	RootComponent = BuildingMesh_;

	// Default initialization of stats via Constructor of struct
	// (MaxHealth, AttackDamage, AttackRange, MoveSpeed)
	Stats_ = FEntityStats( 100, 0, 0.0f, 0.0f );
}

void ABuilding::BeginPlay()
{
	Super::BeginPlay();
}

float ABuilding::TakeDamage( float damageAmount, FDamageEvent const& damageEvent, AController* eventInstigator, AActor* damageCauser )
{
	// Call super to handle generic logic
	const float ActualDamage = Super::TakeDamage( damageAmount, damageEvent, eventInstigator, damageCauser );

	// Standard: spaces around conditions
	if ( ActualDamage > 0.0f )
	{
		// Convert float damage to int for your system
		const int32 IntDamage = FMath::RoundToInt( ActualDamage );

		Stats_.ApplyDamage( IntDamage );

		if  ( !Stats_.IsAlive() )
		{
			// Logic for destruction (effects, removal, etc.)
			Destroy();
		}
	}

	return ActualDamage;
}

const FEntityStats& ABuilding::GetStats() const
{
	return Stats_;
}

bool ABuilding::IsDestroyed() const
{
	return !Stats_.IsAlive();
}