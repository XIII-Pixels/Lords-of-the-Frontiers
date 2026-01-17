#pragma once

#include "Entity.h"
#include "EntityStats.h"
#include "Selectable.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Building.generated.h"

UCLASS( Abstract )
class LORDS_FRONTIERS_API ABuilding : public APawn, public IEntity, public ISelectable
{
	GENERATED_BODY()

public:
	ABuilding();

	virtual float TakeDamage(
	    float damageAmount, const struct FDamageEvent& damageEvent, AController* eventInstigator, AActor* damageCauser
	) override;

	UFUNCTION( BlueprintPure, Category = "Settings|Stats" )
	bool IsDestroyed() const;

	virtual FString GetNameBuild();

	virtual FEntityStats& Stats() override;

	virtual ETeam Team() override;

	virtual void TakeDamage(float damage) override;

	virtual void OnSelected_Implementation() override;

	virtual void OnDeselected_Implementation() override;

	virtual bool CanBeSelected_Implementation() const override;

	virtual FVector GetSelectionLocation_Implementation() const override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Components" )
	UStaticMeshComponent* BuildingMesh_;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Stats" )
	FEntityStats Stats_;
};
