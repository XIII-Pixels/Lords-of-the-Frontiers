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
UENUM( BlueprintType )
enum class EPortalHidePolicy : uint8
{
	HideAfterLastSpawn UMETA( DisplayName = "Hide After Last Spawn" ),
	KeepUntilWaveEnd UMETA( DisplayName = "Keep Until Wave End" ),
};

USTRUCT( BlueprintType )
struct FPortalVisualConfig
{
	GENERATED_BODY()

	UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = "Settings|Portal" )
	bool bEnabled = true;

	UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = "Settings|Portal" )
	TObjectPtr<UStaticMesh> PortalMesh = nullptr;

	UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = "Settings|Portal" )
	TArray<TObjectPtr<UMaterialInterface>> OverrideMaterials;

	UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = "Settings|Portal" )
	FVector RelativeLocation = FVector::ZeroVector;

	UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = "Settings|Portal" )
	FRotator RelativeRotation = FRotator::ZeroRotator;

	UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = "Settings|Portal" )
	FVector RelativeScale = FVector( 1.f );

	UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = "Settings|Portal" )
	bool bShowInBuildPhase = true;

	UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = "Settings|Portal" )
	EPortalHidePolicy HidePolicy = EPortalHidePolicy::HideAfterLastSpawn;
};

UCLASS( Blueprintable )
class LORDS_FRONTIERS_API AEnemyGroupSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	AEnemyGroupSpawnPoint();

	UPROPERTY(
	    EditInstanceOnly, BlueprintReadWrite, Category = "Settings|Spawn", meta = ( DisplayName = "Spawn Point ID" )
	)
	FName SpawnPointId = NAME_None;

	UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = "Settings|Spawn" )
	bool bUseActorTagMatching = false;

	// Sector that this portal belongs to (e.g. N, NE, E ...). Used by the
	// infinite mode builder to pick portals by direction. Leave NAME_None if
	// the portal should not participate in infinite mode.
	UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = "Settings|InfiniteMode" )
	FName Sector = NAME_None;

	// Hint for the builder: this portal is allowed to spawn boss-tier enemies.
	// Disable for tight corridors where a huge boss would get stuck.
	UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = "Settings|InfiniteMode" )
	bool bAllowBoss = true;

	// Visual components (arrow + billboard) to help designer see orientation in
	// editor
	UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = "Settings|Portal" )
	FPortalVisualConfig PortalVisualConfig;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Components" )
	TObjectPtr<UArrowComponent> ArrowComponent;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Components" )
	TObjectPtr<UBillboardComponent> SpriteComponent;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Components" )
	TObjectPtr<UStaticMeshComponent> PortalMeshComponent;

	UFUNCTION( BlueprintPure, Category = "Settings|Spawn" )
	FTransform GetSpawnTransform() const
	{
		return GetActorTransform();
	}

	UFUNCTION( BlueprintPure, Category = "Settings|Spawn" )
	FVector GetSpawnForwardVector() const
	{
		return GetActorForwardVector();
	}

	UFUNCTION( BlueprintCallable, Category = "Settings|Spawn" )
	bool MatchesId( const FName& id ) const;

	UFUNCTION( BlueprintCallable, Category = "Settings|Spawn", meta = ( WorldContext = "WorldContextObject" ) )
	static AEnemyGroupSpawnPoint* FindSpawnPointById( UObject* WorldContextObject, const FName& id );

	UFUNCTION( BlueprintCallable, Category = "Settings|Spawn", meta = ( WorldContext = "WorldContextObject" ) )
	static void
	FindAllSpawnPointsById( UObject* WorldContextObject, const FName& id, TArray<AEnemyGroupSpawnPoint*>& outFound );

	UFUNCTION( BlueprintCallable, Category = "Settings|Portal" )
	void ApplyPortalVisualConfig();

	UFUNCTION( BlueprintCallable, Category = "Settings|Portal" )
	void SetPortalVisible( bool bVisible, bool bDisableCollision = true );

	UFUNCTION( BlueprintPure, Category = "Settings|Portal" )
	bool IsPortalVisible() const
	{
		return PortalMeshComponent && PortalMeshComponent->IsVisible();
	}

protected:
	virtual void OnConstruction( const FTransform& Transform ) override;
	virtual void BeginPlay() override;
};