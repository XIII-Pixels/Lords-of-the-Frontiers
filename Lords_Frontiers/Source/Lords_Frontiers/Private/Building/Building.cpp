#include "Building/Building.h"

#include "Cards/CardSubsystem.h"
#include "Core/CoreManager.h"
#include "VFX/EntityVFXConfig.h"
#include "Lords_Frontiers/Public/Resources/EconomyComponent.h"
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

	StaticMeshComponent_ = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "StaticMesh" ) );
	StaticMeshComponent_->SetCollisionEnabled( ECollisionEnabled::NoCollision );
	StaticMeshComponent_->SetCollisionResponseToAllChannels( ECR_Ignore );
	StaticMeshComponent_->SetupAttachment( RootComponent );

	SkeletalMeshComponent_ = CreateDefaultSubobject<USkeletalMeshComponent>( TEXT( "SkeletalMesh" ) );
	SkeletalMeshComponent_->SetCollisionEnabled( ECollisionEnabled::NoCollision );
	SkeletalMeshComponent_->SetCollisionResponseToAllChannels( ECR_Ignore );
	SkeletalMeshComponent_->SetupAttachment( RootComponent );
}

void ABuilding::BeginPlay()
{
	Super::BeginPlay();

	Stats_.SetHealth( Stats_.MaxHealth() );

	// Save original maintenance for card reset on game restart
	OriginalMaintenanceCost_ = MaintenanceCost_;

	ActivateBuildingMesh();

	SkeletalMeshComponent_->SetPlayRate( FMath::RandRange( AnimationRateRange_.X, AnimationRateRange_.Y ) );

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

	ResolveVFXDefaults();
	SpawnConstructionVFX();
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
	ResolvedConstructionVFX_ = nullptr;
	ResolvedConstructionDelay_ = 0.0f;

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

				if ( classOverride->ConstructionVFX )
				{
					ResolvedConstructionVFX_ = classOverride->ConstructionVFX;
				}

				if ( classOverride->ConstructionDelay >= 0.0f )
				{
					ResolvedConstructionDelay_ = classOverride->ConstructionDelay;
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

			if ( !ResolvedConstructionVFX_ )
			{
				ResolvedConstructionVFX_ = config->DefaultBuildingConstructionVFX;
			}

			if ( ResolvedConstructionDelay_ <= 0.0f )
			{
				ResolvedConstructionDelay_ = config->DefaultConstructionDelay;
			}
		}
	}
}

void ABuilding::SpawnConstructionVFX()
{
	if ( !ResolvedConstructionVFX_ )
	{
		return;
	}

	if ( ResolvedConstructionDelay_ > 0.0f )
	{
		GetWorldTimerManager().SetTimer(
		    ConstructionVFXTimerHandle_,
		    [this]()
		    {
			    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			        GetWorld(), ResolvedConstructionVFX_, GetActorLocation(), GetActorRotation()
			    );
		    },
		    ResolvedConstructionDelay_, false
		);
	}
	else
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		    GetWorld(), ResolvedConstructionVFX_, GetActorLocation(), GetActorRotation()
		);
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

	ActivateRuinsMesh();

	if ( CollisionComponent_ )
	{
		CollisionComponent_->SetCollisionEnabled( ECollisionEnabled::NoCollision );
		CollisionComponent_->SetCollisionResponseToAllChannels( ECR_Ignore );
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

void ABuilding::ActivateBuildingMesh()
{
	if ( SkeletalMeshComponent_->GetSkeletalMeshAsset() )
	{
		StaticMeshComponent_->SetVisibility( false );
		SkeletalMeshComponent_->SetVisibility( true );
	}
	else
	{
		StaticMeshComponent_->SetStaticMesh( BuildingMesh_ );
		SkeletalMeshComponent_->SetVisibility( false );
		StaticMeshComponent_->SetVisibility( true );
	}
}

void ABuilding::ActivateRuinsMesh()
{
	if ( RuinedMesh_ )
	{
		StaticMeshComponent_->SetStaticMesh( RuinedMesh_ );
		StaticMeshComponent_->SetRenderCustomDepth( false );
		SkeletalMeshComponent_->SetVisibility( false );
		StaticMeshComponent_->SetVisibility( true );
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
		StaticMeshComponent_->SetRenderCustomDepth( true );
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

	StaticMeshComponent_->SetRenderCustomDepth( false );
}

bool ABuilding::CanBeSelected_Implementation() const
{
	return Stats_.IsAlive();
}

FVector ABuilding::GetSelectionLocation_Implementation() const
{
	return StaticMeshComponent_->Bounds.Origin;
}

void ABuilding::EndPlay( const EEndPlayReason::Type endPlayReason )
{
	GetWorldTimerManager().ClearTimer( RuinTimerHandle_ );
	GetWorldTimerManager().ClearTimer( ConstructionVFXTimerHandle_ );

	if ( EconomyComponent_ )
	{
		EconomyComponent_->UnregisterBuilding( this );
	}
	Super::EndPlay( endPlayReason );
}

void ABuilding::RestoreFromRuins()
{
	if ( !bIsRuined_ )
	{
		return;
	}

	bIsRuined_ = false;

	ActivateBuildingMesh();

	Stats_.SetHealth( Stats_.MaxHealth() );

	if ( CollisionComponent_ )
	{
		CollisionComponent_->SetCollisionEnabled( ECollisionEnabled::QueryAndPhysics );
		CollisionComponent_->SetCollisionResponseToAllChannels( ECR_Block );
	}

	ActivateBuildingMesh();
}

void ABuilding::FullRestore()
{
	if ( bIsRuined_ )
	{
		bIsRuined_ = false;

		ActivateBuildingMesh();

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
	MaintenanceCost_.ModifyByType( type, delta );
}

void ABuilding::ModifyMaintenanceCostAll( int32 delta )
{
	for ( EResourceType type : CardTypeHelpers::GetAllResourceTypes() )
	{
		MaintenanceCost_.ModifyByType( type, delta );
	}
}

void ABuilding::ResetMaintenanceCostToDefaults()
{
	MaintenanceCost_ = OriginalMaintenanceCost_;
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
int32 ABuilding::GetBuildingTotalCostGold() const
{
	int32 total = 0;
	total += BuildingCost_.Gold;
	total += BuildingCost_.Food;
	total += BuildingCost_.Population;
	total += BuildingCost_.Progress;
	return total;
}

int32 ABuilding::GetRelocationGoldCost() const
{
	return FMath::Max( 0, RelocationCost_.Gold );
}

FResourceProduction ABuilding::GetRelocationCost() const
{
	return RelocationCost_;
}

FResourceProduction ABuilding::GetDemolitionRefund() const
{
	return DemolitionRefund_;
}