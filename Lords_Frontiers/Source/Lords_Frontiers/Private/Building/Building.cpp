#include "Building/Building.h"
#include "Lords_Frontiers/Public/Resources/EconomyComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Cards/CardSubsystem.h"
#include "UI/Widgets/HealthBarWidget.h"
#include "Lords_Frontiers/Public/UI/HealthBarManager.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
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

	// Save original maintenance for card reset on game restart
	OriginalMaintenanceCost_ = MaintenanceCost;

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

	if ( UWidgetComponent* wc = FindComponentByClass<UWidgetComponent>() )
	{
		wc->SetVisibility( false );
		wc->SetHiddenInGame( true );
		if ( UUserWidget* uw = wc->GetUserWidgetObject() )
		{
			if ( UHealthBarWidget* hbw = Cast<UHealthBarWidget>( uw ) )
			{
				hbw->BindToActor( this );
			}
		}
	}
	if ( UCardSubsystem* cardSubsystem = UCardSubsystem::Get( this ) )
	{
		cardSubsystem->OnBuildingPlaced( this );
	}

	UWorld* world = GetWorld();

	world->GetTimerManager().SetTimerForNextTick( FTimerDelegate::CreateLambda(
	    [this]()
	    {
		    AHealthBarManager* found = this->CacheHealthBarManager();

		    if ( IsValid( found ) )
		    {
			    UE_LOG(
			        LogTemp, Log,
			        TEXT( "ABuilding::BeginPlay: HealthBarManager cached successfully for %s -> %s (ptr=%p)" ),
			        *GetNameSafe( this ), *GetNameSafe( found ), found
			    );

			    found->RegisterActor( this, this->HealthBarWorldOffset );

			    UE_LOG(
			        LogTemp, Verbose, TEXT( "ABuilding::BeginPlay: Registered offset %s for %s" ),
			        *HealthBarWorldOffset.ToString(), *GetNameSafe( this )
			    );
		    }
		    else
		    {
			    UE_LOG(
			        LogTemp, Warning,
			        TEXT( "ABuilding::BeginPlay: HealthBarManager NOT found / NOT cached yet for %s" ),
			        *GetNameSafe( this )
			    );
		    }
	    }
	) );
}
void ABuilding::OnDeath()
{
	if ( bIsRuined_ )
	{
		return;
	}

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

int ABuilding::GetCurrentHealth() const
{
	return Stats_.Health();
}
int ABuilding::GetMaxHealth() const
{
	return Stats_.MaxHealth();
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

	AHealthBarManager* manager = CachedHealthBarManager_.Get();

	if ( !IsValid( manager ) )
	{
		manager = CacheHealthBarManager();
	}

	if ( IsValid( manager ) )
	{
		manager->OnActorHealthChanged( this, static_cast<int32>( GetCurrentHealth() ), static_cast<int32>( GetMaxHealth() ) );
	}
	else
	{
		UE_LOG(
		    LogTemp, Warning,
		    TEXT( "ABuilding::TakeDamage: HealthBarManager not available for actor %s � skipping UI update" ),
		    *GetNameSafe( this )
		);
	}

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
	if ( APlayerController* PC = UGameplayStatics::GetPlayerController( GetWorld(), 0 ) )
	{
		if ( UEconomyComponent* Eco = PC->FindComponentByClass<UEconomyComponent>() )
		{
			Eco->UnregisterBuilding( this );
		}
	}
	if ( UWorld* world = GetWorld() )
	{
		if ( AHealthBarManager* manager = Cast<AHealthBarManager>( UGameplayStatics::GetActorOfClass( world, AHealthBarManager::StaticClass() ) ) )
		{
			manager->UnregisterActor( this );
		}
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

	OnBuildingHealthChanged.Broadcast( this->GetCurrentHealth(), this->GetMaxHealth() );

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

	OnBuildingHealthChanged.Broadcast( this->GetCurrentHealth(), this->GetMaxHealth() );
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

AHealthBarManager* ABuilding::CacheHealthBarManager()
{
	CachedHealthBarManager_.Reset();

	UWorld* World = GetWorld();
	if ( !World )
	{
		UE_LOG(
		    LogTemp, Error, TEXT( "CacheHealthBarManager: GetWorld() == nullptr (Actor=%s)" ), *GetNameSafe( this )
		);
		return nullptr;
	}

	AActor* Found = UGameplayStatics::GetActorOfClass( World, AHealthBarManager::StaticClass() );

	UE_LOG(
	    LogTemp, Verbose, TEXT( "CacheHealthBarManager: GetActorOfClass -> ptr=%p name=%s class=%s" ), Found,
	    *GetNameSafe( Found ), Found ? *Found->GetClass()->GetName() : TEXT( "null" )
	);

	if ( !Found )
	{
		return nullptr;
	}

	if ( Found->HasAnyFlags( RF_ClassDefaultObject ) )
	{
		UE_LOG(
		    LogTemp, Error,
		    TEXT( "CacheHealthBarManager: GetActorOfClass returned CDO (Default__), ptr=%p class=%s name=%s" ), Found,
		    *GetNameSafe( Found->GetClass() ), *GetNameSafe( Found )
		);
		return nullptr;
	}

	if ( !Found->IsA( AHealthBarManager::StaticClass() ) )
	{
		UE_LOG(
		    LogTemp, Error,
		    TEXT( "CacheHealthBarManager: Found actor is NOT AHealthBarManager: ptr=%p class=%s name=%s" ), Found,
		    *GetNameSafe( Found->GetClass() ), *GetNameSafe( Found )
		);
		return nullptr;
	}

	AHealthBarManager* Mgr = Cast<AHealthBarManager>( Found );
	if ( !IsValid( Mgr ) )
	{
		UE_LOG( LogTemp, Error, TEXT( "CacheHealthBarManager: Cast succeeded but object is invalid (ptr=%p)" ), Found );
		return nullptr;
	}

	CachedHealthBarManager_ = Mgr;
	UE_LOG(
	    LogTemp, Log, TEXT( "CacheHealthBarManager: cached HealthBarManager = %s (ptr=%p)" ), *GetNameSafe( Mgr ), Mgr
	);
	return Mgr;
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
