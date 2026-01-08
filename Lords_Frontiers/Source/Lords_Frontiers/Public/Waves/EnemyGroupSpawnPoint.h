#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "EnemyGroupSpawnPoint.generated.h"

class UArrowComponent;
class UBillboardComponent;

/*
* (Artyom)
  Actor-point used as spawn location & orientation for enemy groups
  - Placed on level by designer
  - Has optional SpawnPointId (FName) that Wave / WaveManager can use to lookup
the point
  - Optionally uses ActorTags matching for lookup (bUseActorTagMatching)
 */
UCLASS( Blueprintable )
class LORDS_FRONTIERS_API AEnemyGroupSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	AEnemyGroupSpawnPoint();

	// Unique identifier for this spawn point. Can be used by Wave/WaveManager to
	// find the point
	UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = "Spawn", meta = ( DisplayName = "Spawn Point ID" ) )
	FName SpawnPointId = NAME_None;

	// If true, the actor's tags will also be considered when searching by id
	// Use if you prefer to find spawn points by Actor->Tags in the level
	UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = "Spawn" )
	bool bUseActorTagMatching = false;

	// Visual components (arrow + billboard) to help designer see orientation in
	// editor
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Components" )
	TObjectPtr<UArrowComponent> ArrowComponent;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Components" )
	TObjectPtr<UBillboardComponent> SpriteComponent;

public:
	// Return transform to spawn enemies (world transform of this actor)
	UFUNCTION( BlueprintPure, Category = "Spawn" )
	FTransform GetSpawnTransform() const
	{
		return GetActorTransform();
	}

	// Return forward vector (spawn facing) in world space.
	UFUNCTION( BlueprintPure, Category = "Spawn" )
	FVector GetSpawnForwardVector() const
	{
		return GetActorForwardVector();
	}

	// Does this spawn point match the given id/name? (checks SpawnPointId and
	// optionally ActorTags)
	UFUNCTION( BlueprintCallable, Category = "Spawn" )
	bool MatchesId( const FName& id ) const;

	// Helper: find first spawn point in world that matches id
	// Returns nullptr if none found. Useful for WaveManager: pass GetWorld() as
	// WorldContext
	UFUNCTION( BlueprintCallable, Category = "Spawn", meta = ( WorldContext = "WorldContextObject" ) )
	static AEnemyGroupSpawnPoint* FindSpawnPointById( UObject* worldContextObject, const FName& id );

	// Helper: find all spawn points that match id (may be multiple when using
	// tags)
	UFUNCTION( BlueprintCallable, Category = "Spawn", meta = ( WorldContext = "WorldContextObject" ) )
	static void
	FindAllSpawnPointsById( UObject* worldContextObject, const FName& id, TArray<AEnemyGroupSpawnPoint*>& outFound );
};
