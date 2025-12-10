#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EntityStats.h"
#include "Building.generated.h"

// Abstract base class for all buildings.
// Contains visual representation and entity stats.
UCLASS(Abstract)
class LORDS_FRONTIERS_API ABuilding : public AActor
{
	GENERATED_BODY()

public:
	ABuilding();

public:
	// Override standard UE damage handling to pass it to EntityStats.
	virtual float TakeDamage( float damageAmount, struct FDamageEvent const& damageEvent, AController* eventInstigator, AActor* damageCauser ) override;

	// Returns current stats (const version)
	UFUNCTION(BlueprintPure, Category = "Stats")
	const FEntityStats& GetStats() const;

	// Check if building is destroyed
	UFUNCTION(BlueprintPure, Category = "Stats")
	bool IsDestroyed() const;

protected:
	virtual void BeginPlay() override;

protected:
	// Visual representation of the building
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* BuildingMesh_;

	// Stats struct (Health, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FEntityStats Stats_;
};