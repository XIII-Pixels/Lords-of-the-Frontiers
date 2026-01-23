#pragma once

#include "EntityStats.h"
#include "Selectable.h"
#include "Lords_Frontiers/Public/Resources/GameResource.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Building.generated.h"

UCLASS( Abstract )
class LORDS_FRONTIERS_API ABuilding : public AActor, public ISelectable
{
	GENERATED_BODY()

public:
	ABuilding();

	virtual float TakeDamage(
	    float damageAmount, const struct FDamageEvent& damageEvent, AController* eventInstigator, AActor* damageCauser
	) override;

	UFUNCTION( BlueprintPure, Category = "Settings|Stats" )
	const FEntityStats& GetStats() const;

	UFUNCTION( BlueprintPure, Category = "Settings|Stats" )
	bool IsDestroyed() const;

	virtual FString GetNameBuild();

	virtual void OnSelected_Implementation() override;

	virtual void OnDeselected_Implementation() override;

	virtual bool CanBeSelected_Implementation() const override;

	virtual FVector GetSelectionLocation_Implementation() const override;

	const FResourceProduction& GetMaintenanceCost() const
	{
		return MaintenanceCost;
	}

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Economy" )
	FResourceProduction MaintenanceCost;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Setting|Components" )
	UStaticMeshComponent* BuildingMesh_;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Setting|Stats" )
	FEntityStats Stats_;
};