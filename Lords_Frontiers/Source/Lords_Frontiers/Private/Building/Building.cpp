#include "Building/Building.h"

#include "Components/BoxComponent.h"
#include "Utilities/TraceChannelMappings.h"

ABuilding::ABuilding()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent_ = CreateDefaultSubobject<UBoxComponent>( TEXT( "BoxCollision" ) );
	CollisionComponent_->SetCollisionObjectType( ECC_Entity );
	SetRootComponent( CollisionComponent_ );

	BuildingMesh_ = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "BuildingMesh" ) );
	BuildingMesh_->SetupAttachment( RootComponent );
}

void ABuilding::BeginPlay()
{
	Super::BeginPlay();
}

void ABuilding::OnDeath()
{
	// Do not destroy building
	// Replace building with ruins
	// Disable component ticks in children
}

FEntityStats& ABuilding::Stats()
{
	return Stats_;
}

ETeam ABuilding::Team()
{
	return Stats_.Team();
}

void ABuilding::TakeDamage( float damage )
{
	if ( !Stats_.IsAlive() )
	{
		return;
	}

	Stats_.ApplyDamage( damage );
	if ( !Stats_.IsAlive() )
	{
		OnDeath();
	}
}

bool ABuilding::IsDestroyed() const
{
	return !Stats_.IsAlive();
}

FString ABuilding::GetNameBuild()
{
	return TEXT( "Build" );
}

void ABuilding::OnSelected_Implementation()
{
	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage(
		    -1, 1.0f, FColor::Green, FString::Printf( TEXT( "OnSelected: %s" ), *GetName() )
		);
	}

	if ( BuildingMesh_ )
	{
		BuildingMesh_->SetRenderCustomDepth( true );
	}
}

void ABuilding::OnDeselected_Implementation()
{
	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage(
		    -1, 1.0f, FColor::Red, FString::Printf( TEXT( "OnDeselected: %s" ), *GetName() )
		);
	}

	if ( BuildingMesh_ )
	{
		BuildingMesh_->SetRenderCustomDepth( false );
	}
}

bool ABuilding::CanBeSelected_Implementation() const
{
	return Stats_.IsAlive();
}

FVector ABuilding::GetSelectionLocation_Implementation() const
{
	if ( BuildingMesh_ )
	{
		return BuildingMesh_->Bounds.Origin;
	}

	return GetActorLocation();
}
