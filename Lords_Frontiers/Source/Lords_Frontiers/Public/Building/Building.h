#pragma once

#include "Cards/CardTypes.h"
#include "Entity.h"
#include "EntityStats.h"
#include "Lords_Frontiers/Public/Resources/GameResource.h"
#include "Selectable.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Building.generated.h"

class UBoxComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnBuildingHealthChanged, int32, CurrentHealth, int32, MaxHealth );

UCLASS( Abstract )
class LORDS_FRONTIERS_API ABuilding : public APawn, public IEntity, public ISelectable
{
	GENERATED_BODY()

public:
	ABuilding();

	bool IsDestroyed() const;

	void RestoreFromRuins();

	void FullRestore();

	virtual FString GetNameBuild();

	virtual FEntityStats& Stats() override;

	virtual ETeam Team() override;

	virtual void TakeDamage( float damage ) override;

	virtual void OnSelected_Implementation() override;

	virtual void OnDeselected_Implementation() override;

	virtual bool CanBeSelected_Implementation() const override;

	virtual FVector GetSelectionLocation_Implementation() const override;

	const FResourceProduction& GetMaintenanceCost() const
	{
		return MaintenanceCost;
	}

	const FResourceProduction& GetBuildingCost() const
	{
		return BuildingCost_;
	}

	/**
	 * Modifies MaintenanceCost for a specific resource type.
	 * @param type - Which resource to modify
	 * @param delta - Value to add (negative = cheaper maintenance)
	 */
	void ModifyMaintenanceCost( EResourceType type, int32 delta );

	/**
	 * Modifies MaintenanceCost for all resource types at once.
	 * @param delta - Value to add to each resource
	 */
	void ModifyMaintenanceCostAll( int32 delta );

	/**
	 * Resets MaintenanceCost to original values from CDO.
	 * Call on game restart to undo all card modifications.
	 */
	void ResetMaintenanceCostToDefaults();

    UPROPERTY( BlueprintAssignable, Category = "Stats|Events" )
	FOnBuildingHealthChanged OnBuildingHealthChanged;

	int GetCurrentHealth() const;

	int GetMaxHealth() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Economy" )
	FResourceProduction MaintenanceCost;

	virtual void OnDeath();

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Visuals" )
	TObjectPtr<UStaticMesh> RuinedMesh_;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|State" )
	bool bIsRuined_ = false;

	UPROPERTY()
	TObjectPtr<UBoxComponent> CollisionComponent_;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Components" )
	UStaticMeshComponent* BuildingMesh_;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Stats" )
	FEntityStats Stats_;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Economy" )
	FResourceProduction BuildingCost_;

	UPROPERTY()
	TObjectPtr<UStaticMesh> DefaultMesh_;

private:
	FResourceProduction OriginalMaintenanceCost_;
	UPROPERTY( EditAnywhere, Category = "Settings|Visuals" )
	FText BuildingDisplayName_;
};
