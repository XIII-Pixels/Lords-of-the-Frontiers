#include "Lords_Frontiers/Public/Building.h"

ABuilding::ABuilding()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create and set root component
	BuildingMesh_ = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "BuildingMesh" ) );
	RootComponent = BuildingMesh_;

	// Default initialization of stats (can be overwritten in Blueprints)
	Stats_ = FEntityStats( 100, 0, 0.0f, 0.0f ); // 100 HP, 0 DMG (Buildings usually don't move or attack by default)
}

void ABuilding::BeginPlay()
{
	Super::BeginPlay();
}

float ABuilding::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// Call super to handle generic logic
	const float ActualDamage = Super::TakeDamage( DamageAmount, DamageEvent, EventInstigator, DamageCauser );

	if ( ActualDamage > 0.0f )
	{
		// Convert float damage to int for system
		const int32 IntDamage = FMath::RoundToInt( ActualDamage );
		Stats_.ApplyDamage( IntDamage );

		if ( !Stats_.IsAlive() )
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