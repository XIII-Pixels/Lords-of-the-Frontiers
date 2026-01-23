#include "Building/Building.h"
#include "Lords_Frontiers/Public/Resources/EconomyComponent.h"
#include "Kismet/GameplayStatics.h"

ABuilding::ABuilding()
{

	PrimaryActorTick.bCanEverTick = false;

	BuildingMesh_ = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "BuildingMesh" ) );
	RootComponent = BuildingMesh_;

	BuildingMesh_->SetCollisionResponseToChannel( ECollisionChannel::ECC_GameTraceChannel1, ECR_Ignore );

	Stats_ = FEntityStats( 100, 0, 0.0f );
}

void ABuilding::BeginPlay()
{
	Super::BeginPlay();

	if ( APlayerController* PC = UGameplayStatics::GetPlayerController( GetWorld(), 0 ) )
	{
		if ( UEconomyComponent* Eco = PC->FindComponentByClass<UEconomyComponent>() )
		{
			Eco->RegisterBuilding( this );
		}
	}
}

float ABuilding::TakeDamage(
    float damageAmount, const FDamageEvent& damageEvent, AController* eventInstigator, AActor* damageCauser
)
{
	// Call super to handle generic logic
	const float ActualDamage = Super::TakeDamage( damageAmount, damageEvent, eventInstigator, damageCauser );

	// Standard: spaces around conditions
	if ( ActualDamage > 0.0f )
	{
		// Convert float damage to int for your system
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