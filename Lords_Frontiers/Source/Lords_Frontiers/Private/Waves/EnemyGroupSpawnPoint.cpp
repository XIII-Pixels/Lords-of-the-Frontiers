#include "Lords_Frontiers/Public/Waves/EnemyGroupSpawnPoint.h"
#include "Components/ArrowComponent.h"
#include "Components/BillboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetStringLibrary.h"
#include "Engine/World.h"

AEnemyGroupSpawnPoint::AEnemyGroupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root - keep actor transform as spawn transform
	SetRootComponent ( CreateDefaultSubobject <USceneComponent> ( TEXT ( "Root" ) ) );

	// Arrow to show forward direction in editor
	ArrowComponent = CreateDefaultSubobject <UArrowComponent> ( TEXT ( "Arrow" ) );
	ArrowComponent -> SetupAttachment ( RootComponent );
	ArrowComponent -> ArrowSize = 1.0f;
	ArrowComponent -> bIsScreenSizeScaled = true;

	// Small sprite so designer can see it in editor
	SpriteComponent = CreateDefaultSubobject <UBillboardComponent> ( TEXT ( "Sprite" ) );
	SpriteComponent -> SetupAttachment ( RootComponent ) ;

	SpawnPointId = NAME_None;
	bUseActorTagMatching = false;

	bIsEditorOnlyActor = false;
}

bool AEnemyGroupSpawnPoint::MatchesId ( const FName& id ) const
{
	if ( id.IsNone () )
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

AEnemyGroupSpawnPoint* AEnemyGroupSpawnPoint::FindSpawnPointById ( UObject* worldContextObject, const FName& id )
{
	if (!worldContextObject)
	{
		return nullptr;
	}

	UWorld* world = worldContextObject->GetWorld ();
	if (!world)
	{
		return nullptr;
	}

	// get all actors
	TArray <AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass ( world, AEnemyGroupSpawnPoint::StaticClass (), foundActors );

	for (AActor* actor : foundActors)
	{
		AEnemyGroupSpawnPoint* candidate = Cast <AEnemyGroupSpawnPoint> ( actor );
		if ( !candidate ) continue;

		// 1) direct id match
		if ( !id.IsNone() && candidate -> SpawnPointId == id )
		{
			return candidate;
		}

		// 2) optional: match by actor tags if enabled
		if ( candidate -> bUseActorTagMatching )
		{
			for ( const FName& tag : candidate->Tags )
			{
				if ( tag == id )
				{
					return candidate;
				}
			}
		}
	}

	return nullptr;
}

void AEnemyGroupSpawnPoint::FindAllSpawnPointsById ( UObject* worldContextObject, const FName& id, TArray <AEnemyGroupSpawnPoint*>& outFound )
{
	outFound.Reset ();

	if ( !worldContextObject || id.IsNone () )
	{
		return;
	}

	UWorld* world = GEngine ? GEngine -> GetWorldFromContextObjectChecked ( worldContextObject ) : nullptr;
	if ( !world )
	{
		return;
	}

	TArray <AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass ( world, AEnemyGroupSpawnPoint::StaticClass (), foundActors );

	for ( AActor* actor : foundActors )
	{
		if ( AEnemyGroupSpawnPoint* spawnPoint = Cast <AEnemyGroupSpawnPoint> ( actor ) )
		{
			if ( spawnPoint -> MatchesId ( id ) )
			{
				outFound.Add ( spawnPoint );
			}
		}
	}
}
