#include "Lords_Frontiers/Public/Waves/EnemyGroupSpawnPoint.h"

#include "Components/ArrowComponent.h"
#include "Components/BillboardComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "UObject/ScriptDelegates.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "UObject/ScriptDelegates.h"
#include "Kismet/KismetStringLibrary.h"

AEnemyGroupSpawnPoint::AEnemyGroupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root - keep actor transform as spawn transform
	SetRootComponent( CreateDefaultSubobject<USceneComponent>( TEXT( "Root" ) ) );

	// Arrow to show forward direction in editor
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>( TEXT( "Arrow" ) );
	ArrowComponent->SetupAttachment( RootComponent );
	ArrowComponent->ArrowSize = 1.0f;
	ArrowComponent->bIsScreenSizeScaled = true;

	// Small sprite so designer can see it in editor
	SpriteComponent = CreateDefaultSubobject<UBillboardComponent>( TEXT( "Sprite" ) );
	SpriteComponent->SetupAttachment( RootComponent );

	SpawnPointId = NAME_None;
	bUseActorTagMatching = false;

	bIsEditorOnlyActor = false;

	PortalMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "PortalMeshComponent" ) );
	PortalMeshComponent->SetupAttachment( RootComponent );
	PortalMeshComponent->SetCollisionEnabled( ECollisionEnabled::NoCollision );
	PortalMeshComponent->SetGenerateOverlapEvents( false );
	PortalMeshComponent->SetHiddenInGame( true );
	PortalMeshComponent->SetVisibility( false );
}

void AEnemyGroupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	ApplyPortalVisualConfig();
	SetPortalVisible( false );
}

void AEnemyGroupSpawnPoint::ApplyPortalVisualConfig()
{
	if ( !PortalMeshComponent )
	{
		return;
	}

	if ( !PortalVisualConfig.bEnabled )
	{
		SetPortalVisible( false );
		return;
	}

	if ( PortalVisualConfig.PortalMesh )
	{
		PortalMeshComponent->SetStaticMesh( PortalVisualConfig.PortalMesh );
	}

	for ( int32 i = 0; i < PortalVisualConfig.OverrideMaterials.Num(); ++i )
	{
		if ( PortalVisualConfig.OverrideMaterials[i] )
		{
			PortalMeshComponent->SetMaterial( i, PortalVisualConfig.OverrideMaterials[i] );
		}
	}

	PortalMeshComponent->SetRelativeLocation( PortalVisualConfig.RelativeLocation );
	PortalMeshComponent->SetRelativeRotation( PortalVisualConfig.RelativeRotation );
	PortalMeshComponent->SetRelativeScale3D( PortalVisualConfig.RelativeScale );
}

void AEnemyGroupSpawnPoint::SetPortalVisible( bool bVisible, bool bDisableCollision )
{
	if ( !PortalMeshComponent || !PortalVisualConfig.bEnabled )
	{
		return;
	}

	PortalMeshComponent->SetHiddenInGame( !bVisible );
	PortalMeshComponent->SetVisibility( bVisible, true );

	if ( bVisible )
	{
		PortalMeshComponent->SetCollisionEnabled( ECollisionEnabled::NoCollision );
	}
	else
	{
		PortalMeshComponent->SetCollisionEnabled(
		    bDisableCollision ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryOnly
		);
	}
}
void AEnemyGroupSpawnPoint::OnConstruction( const FTransform& Transform )
{
	Super::OnConstruction( Transform );
	ApplyPortalVisualConfig();
}

bool AEnemyGroupSpawnPoint::MatchesId( const FName& id ) const
{
	if ( id.IsNone() )
	{
		return false;
	}

	// primary match by explicit SpawnPointId
	if ( SpawnPointId == id )
	{
		return true;
	}

	// match by ActorTags (if enabled)
	if ( bUseActorTagMatching )
	{
		for ( const FName& tag : Tags )
		{
			if ( tag == id )
			{
				return true;
			}
		}
	}

	// Not matched
	return false;
}

AEnemyGroupSpawnPoint* AEnemyGroupSpawnPoint::FindSpawnPointById( UObject* worldContextObject, const FName& id )
{
	if ( !worldContextObject )
	{
		return nullptr;
	}

	UWorld* world = worldContextObject->GetWorld();
	if ( !world )
	{
		return nullptr;
	}

	// get all actors
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass( world, AEnemyGroupSpawnPoint::StaticClass(), foundActors );

	for ( AActor* actor : foundActors )
	{
		AEnemyGroupSpawnPoint* candidate = Cast<AEnemyGroupSpawnPoint>( actor );
		if ( !candidate )
			continue;

		if ( candidate->MatchesId( id ) )
		{
			return candidate;
		}
	}

	return nullptr;
}

void AEnemyGroupSpawnPoint::FindAllSpawnPointsById(
    UObject* worldContextObject, const FName& id, TArray<AEnemyGroupSpawnPoint*>& outFound
)
{
	outFound.Reset();

	if ( !worldContextObject || id.IsNone() )
	{
		return;
	}

	UWorld* world = GEngine ? GEngine->GetWorldFromContextObjectChecked( worldContextObject ) : nullptr;
	if ( !world )
	{
		return;
	}

	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass( world, AEnemyGroupSpawnPoint::StaticClass(), foundActors );

	for ( AActor* actor : foundActors )
	{
		if ( AEnemyGroupSpawnPoint* spawnPoint = Cast<AEnemyGroupSpawnPoint>( actor ) )
		{
			if ( spawnPoint->MatchesId( id ) )
			{
				outFound.Add( spawnPoint );
			}
		}
	}
}
