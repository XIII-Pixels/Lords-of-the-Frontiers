#pragma once

#include "EntityStats.h"
#include "Selectable.h"

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

protected:
	virtual void BeginPlay() override;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Setting|Components" )
	UStaticMeshComponent* BuildingMesh_;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Setting|Stats" )
	FEntityStats Stats_;
};