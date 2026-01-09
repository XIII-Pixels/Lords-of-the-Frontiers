#pragma once

#include "CoreMinimal.h"
#include "EntityStats.h"
#include "GameFramework/Actor.h"
#include "Selectable.h"

#include "Building.generated.h"

enum class EBuildingCategory : uint8;
struct FCardModifiers;

UCLASS( Abstract )
class LORDS_FRONTIERS_API ABuilding : public AActor, public ISelectable
{
	GENERATED_BODY()

  public:
	ABuilding();

	virtual float TakeDamage(
	    float damageAmount, struct FDamageEvent const& damageEvent, AController* eventInstigator, AActor* damageCauser
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

	virtual EBuildingCategory GetBuildingCategory() const;

	UFUNCTION()
	void ApplyUpgrades();

	FCardModifiers GetModifiers() const;
  protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Setting|Components" )
	UStaticMeshComponent* BuildingMesh_;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Setting|Stats" )
	FEntityStats Stats_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Stats")
	FEntityStats BaseStats_;


};