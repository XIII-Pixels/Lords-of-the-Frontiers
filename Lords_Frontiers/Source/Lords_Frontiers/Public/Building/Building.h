#pragma once

#include "Entity.h"
#include "EntityStats.h"
#include "Selectable.h"
#include "Lords_Frontiers/Public/Resources/GameResource.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Building.generated.h"

class UBoxComponent;

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

	virtual void TakeDamage(float damage) override;

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

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Visuals" )
	FText BuildingDisplayName_;
};
