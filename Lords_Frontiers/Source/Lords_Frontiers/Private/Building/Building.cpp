#include "Building/Building.h"

#include "Cards/CardSubsystem.h"

#include "Cards/CardSubsystem.h"
#include "Core/CoreManager.h"
#include "Core/EntityVFXConfig.h"
#include "Lords_Frontiers/Public/Resources/EconomyComponent.h"
#include "Utilities/TraceChannelMappings.h"

#include "Components/BoxComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Utilities/TraceChannelMappings.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

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

	// Save original maintenance for card reset on game restart
	OriginalMaintenanceCost_ = MaintenanceCost;

	if ( BuildingMesh_ )
	{
		DefaultMesh_ = BuildingMesh_->GetStaticMesh();
	}

	if ( APlayerController* pc = UGameplayStatics::GetPlayerController( GetWorld(), 0 ) )
	{
		EconomyComponent_ = pc->FindComponentByClass<UEconomyComponent>();
	}

	if ( EconomyComponent_ )
	{
		EconomyComponent_->RegisterBuilding( this );

		EconomyComponent_->RecalculateAndBroadcastNetIncome();
	}

	if ( UCardSubsystem* cardSubsystem = UCardSubsystem::Get( this ) )
	{
		cardSubsystem->OnBuildingPlaced( this );
	}

	if ( EconomyComponent_ )
	{
	}
	ResolveVFXDefaults();
}

void ABuilding::OnDeath()
{
	if ( bIsRuined_ )
	{
		return;
	}

	SpawnDestructionVFX();

	if ( ResolvedRuinDelay_ > 0.0f )
	{
		GetWorldTimerManager().SetTimer( RuinTimerHandle_, this, &ABuilding::FinalizeRuin, ResolvedRuinDelay_, true );
	}
	else
	{
		FinalizeRuin();
	}
}

UNiagaraSystem* ABuilding::GetHitVFX() const
{
	return ResolvedHitVFX_;
}

void ABuilding::ResolveVFXDefaults()
{
	ResolvedHitVFX_ = HitVFX_;
	ResolvedDestructionVFX_ = DestructionVFX_;
	ResolvedRuinDelay_ = 0.0f;

	if ( UCoreManager* core = UCoreManager::Get( this ) )
	{
		if ( const UEntityVFXConfig* config = core->GetEntityVFXConfig() )
		{
			if ( const FBuildingVFXOverride* classOverride = config->BuildingOverrides.Find( GetClass() ) )
			{
				if ( !ResolvedHitVFX_ && classOverride->HitVFX )
				{
					ResolvedHitVFX_ = classOverride->HitVFX;
				}

				if ( !ResolvedDestructionVFX_ && classOverride->DestructionVFX )
				{
					ResolvedDestructionVFX_ = classOverride->DestructionVFX;
				}

				if ( classOverride->RuinDelay >= 0.0f )
				{
					ResolvedRuinDelay_ = classOverride->RuinDelay;
				}
			}

			if ( !ResolvedHitVFX_ )
			{
				ResolvedHitVFX_ = config->DefaultBuildingHitVFX;
			}

			if ( !ResolvedDestructionVFX_ )
			{
				ResolvedDestructionVFX_ = config->DefaultBuildingDestructionVFX;
			}

			if ( ResolvedRuinDelay_ <= 0.0f )
			{
				ResolvedRuinDelay_ = config->DefaultRuinDelay;
			}
		}
	}
}

void ABuilding::SpawnDestructionVFX()
{
	if ( !ResolvedDestructionVFX_ )
	{
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
	    GetWorld(), ResolvedDestructionVFX_, GetActorLocation(), GetActorRotation(), FVector( 1.0f ), true
	);
}

void ABuilding::FinalizeRuin()
{
	bIsRuined_ = true;

	if ( BuildingMesh_ && RuinedMesh_ )
	{
		BuildingMesh_->SetStaticMesh( RuinedMesh_ );
		BuildingMesh_->SetRenderCustomDepth( false );
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

	OnBuildingDied.Broadcast( this );

	if ( EconomyComponent_ )
	{
		EconomyComponent_->RecalculateAndBroadcastNetIncome();
	}

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

const FEntityStats& ABuilding::Stats() const
{
	return Stats_;
}
ETeam ABuilding::Team()
{
	return Stats_.Team();
}

ETeam ABuilding::Team() const
{
	return Stats_.Team();
}

void ABuilding::TakeDamage( int damage )
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
	if ( !BuildingDisplayName_.IsEmpty() )
	{
		return BuildingDisplayName_.ToString();
	}
	FString ClassName = GetClass()->GetName();
	ClassName.RemoveFromEnd( TEXT( "_C" ) );
	ClassName.RemoveFromStart( TEXT( "Default__" ) );

	return ClassName;
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
	GetWorldTimerManager().ClearTimer( RuinTimerHandle_ );

	if ( EconomyComponent_ )
	{
		EconomyComponent_->UnregisterBuilding( this );
	}
	Super::EndPlay( EndPlayReason );
}

void ABuilding::RestoreFromRuins()
{
	if ( !bIsRuined_ )
	{
		return;
	}

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

		if ( EconomyComponent_ )
		{
			EconomyComponent_->RegisterBuilding( this );
		}
		SetCanAffectNavigationGeneration( true );
	}

	Stats_.SetHealth( Stats_.MaxHealth() );
}

// =============================================================================
// Direct card modifiers
// =============================================================================

void ABuilding::ModifyMaintenanceCost( EResourceType type, int32 delta )
{
	MaintenanceCost.ModifyByType( type, delta );
}

void ABuilding::ModifyMaintenanceCostAll( int32 delta )
{
	for ( EResourceType type : CardTypeHelpers::GetAllResourceTypes() )
	{
		MaintenanceCost.ModifyByType( type, delta );
	}
}

void ABuilding::ResetMaintenanceCostToDefaults()
{
	MaintenanceCost = OriginalMaintenanceCost_;
}

UTexture2D* ABuilding::GetBuildingIconFromClass( TSubclassOf<ABuilding> buildingClass )
{
	if ( !buildingClass )
	{
		return nullptr;
	}
	const ABuilding* cdo = buildingClass->GetDefaultObject<ABuilding>();
	if ( cdo )
	{
		return cdo->BuildingIcon;
	}
	return nullptr;
}
