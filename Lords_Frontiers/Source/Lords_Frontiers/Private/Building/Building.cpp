#include "Building/Building.h"
#include "UpgradeSystem/UpgradeManager.h"
#include "UpgradeSystem/Card.h"

ABuilding::ABuilding()
{

	PrimaryActorTick.bCanEverTick = false;

	BuildingMesh_ = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "BuildingMesh" ) );
	RootComponent = BuildingMesh_;

	BuildingMesh_->SetCollisionResponseToChannel( ECollisionChannel::ECC_GameTraceChannel1, ECR_Ignore );

	BaseStats_ = FEntityStats( 100, 0, 0.0f, 0.0f );
}

void ABuilding::BeginPlay()
{
	Super::BeginPlay();

	Stats_ = BaseStats_;

	if (UUpgradeManager* Manager = GetWorld()->GetSubsystem<UUpgradeManager>())
	{
		Manager->OnUpgradeApplied.AddDynamic(this, &ABuilding::ApplyUpgrades);
	}

	ApplyUpgrades();
}

void ABuilding::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UUpgradeManager* Manager = GetWorld()->GetSubsystem<UUpgradeManager>())
	{
		Manager->OnUpgradeApplied.RemoveDynamic(this, &ABuilding::ApplyUpgrades);
	}

	Super::EndPlay(EndPlayReason);
}


float ABuilding::TakeDamage(
    float damageAmount, FDamageEvent const& damageEvent, AController* eventInstigator, AActor* damageCauser
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

void ABuilding::ApplyUpgrades()
{
	Stats_ = BaseStats_;

	if (UUpgradeManager* Manager = GetWorld()->GetSubsystem<UUpgradeManager>())
	{
		FCardModifiers Mods = Manager->GetModifiersForCategory(GetBuildingCategory());

		
		Stats_.MaxHealth = FMath::RoundToInt(BaseStats_.MaxHealth * Mods.HPMultiplier);
		Stats_.Health = Stats_.MaxHealth;  
		Stats_.AttackDamage = FMath::RoundToInt(BaseStats_.AttackDamage * Mods.DamageMultiplier);
		Stats_.AttackRange = BaseStats_.AttackRange * Mods.RangeMultiplier;
	}
}

EBuildingCategory ABuilding::GetBuildingCategory() const
{
	return EBuildingCategory::All;
}

FCardModifiers ABuilding::GetModifiers() const
{
	if (UUpgradeManager* Manager = GetWorld()->GetSubsystem<UUpgradeManager>())
	{
		return Manager->GetModifiersForCategory(GetBuildingCategory());
	}
	return FCardModifiers();
}