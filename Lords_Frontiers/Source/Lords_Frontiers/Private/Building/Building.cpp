#include "Building/Building.h"
#include "Lords_Frontiers/Public/Resources/EconomyComponent.h"
#include "Kismet/GameplayStatics.h"

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

	Stats_.SetHealth( Stats_.MaxHealth() );

	if ( BuildingMesh_ )
	{
		DefaultMesh_ = BuildingMesh_->GetStaticMesh();
	}

	if ( APlayerController* PC = UGameplayStatics::GetPlayerController( GetWorld(), 0 ) )
	{
		if ( UEconomyComponent* Eco = PC->FindComponentByClass<UEconomyComponent>() )
		{
			Eco->RegisterBuilding( this );
		}
	}
}

void ABuilding::OnDeath()
{
	if (bIsRuined_)
	{
		return;
	}
		
	bIsRuined_ = true;

	if ( BuildingMesh_ && RuinedMesh_ )
	{
		BuildingMesh_->SetStaticMesh( RuinedMesh_ );
		BuildingMesh_->SetRenderCustomDepth( false );
	}

	if ( APlayerController* pc = UGameplayStatics::GetPlayerController( GetWorld(), 0 ) )
	{
		if ( UEconomyComponent* eco = pc->FindComponentByClass<UEconomyComponent>() )
		{
			eco->UnregisterBuilding( this );
		}
	}

	if ( CollisionComponent_ )
	{
		CollisionComponent_->SetCollisionEnabled( ECollisionEnabled::NoCollision );
		CollisionComponent_->SetCollisionResponseToAllChannels( ECR_Ignore );
	}

	if ( BuildingMesh_ )
	{
		BuildingMesh_->SetCollisionEnabled( ECollisionEnabled::NoCollision );
		BuildingMesh_->SetCollisionResponseToAllChannels( ECR_Ignore );
	}

	SetCanAffectNavigationGeneration( false );

	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage(
		    -1, 3.0f, FColor::Orange,
		    FString::Printf( TEXT( "Building %s: Collision disabled, enemies can pass." ), *GetName() )
		);
	}
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

void ABuilding::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
	if ( APlayerController* PC = UGameplayStatics::GetPlayerController( GetWorld(), 0 ) )
	{
		if ( UEconomyComponent* Eco = PC->FindComponentByClass<UEconomyComponent>() )
		{
			Eco->UnregisterBuilding( this );
		}
	}
	Super::EndPlay( EndPlayReason );
}

void ABuilding::RestoreFromRuins()
{
	if ( !bIsRuined_ )
		return;

	bIsRuined_ = false;

	if ( BuildingMesh_ && DefaultMesh_ )
	{
		BuildingMesh_->SetStaticMesh( DefaultMesh_ );
	}

	Stats_.SetHealth( Stats_.MaxHealth() );

	if ( CollisionComponent_ )
	{
		CollisionComponent_->SetCollisionEnabled( ECollisionEnabled::QueryAndPhysics );
		CollisionComponent_->SetCollisionResponseToAllChannels( ECR_Block );
	}
	if ( BuildingMesh_ )
	{
		BuildingMesh_->SetCollisionEnabled( ECollisionEnabled::QueryAndPhysics );
		BuildingMesh_->SetCollisionResponseToAllChannels( ECR_Block );
	}

	if ( APlayerController* pc = UGameplayStatics::GetPlayerController( GetWorld(), 0 ) )
	{
		if ( UEconomyComponent* eco = pc->FindComponentByClass<UEconomyComponent>() )
		{
			eco->RegisterBuilding( this );
		}
	}
}

void ABuilding::FullRestore()
{
	if ( bIsRuined_ )
	{
		bIsRuined_ = false;
		if ( BuildingMesh_ && DefaultMesh_ )
		{
			BuildingMesh_->SetStaticMesh( DefaultMesh_ );
		}

		if ( CollisionComponent_ )
		{
			CollisionComponent_->SetCollisionEnabled( ECollisionEnabled::QueryAndPhysics );
			CollisionComponent_->SetCollisionResponseToAllChannels( ECR_Block );
		}

		if ( APlayerController* pc = UGameplayStatics::GetPlayerController( GetWorld(), 0 ) )
		{
			if ( UEconomyComponent* eco = pc->FindComponentByClass<UEconomyComponent>() )
			{
				eco->RegisterBuilding( this );
			}
		}
		SetCanAffectNavigationGeneration( true );
	}

	Stats_.SetHealth( Stats_.MaxHealth() );
}