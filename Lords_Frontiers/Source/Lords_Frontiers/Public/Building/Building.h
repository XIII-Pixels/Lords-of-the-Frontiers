#pragma once

#include "Building/Bonus/BuildingBonusComponent.h"
#include "Entity.h"
#include "EntityStats.h"
#include "Lords_Frontiers/Public/Resources/GameResource.h"
#include "Selectable.h"

#include "CoreMinimal.h"

#include "Building.generated.h"

class UEconomyComponent;
class UBoxComponent;
class UNiagaraSystem;
class UHealthBarConfigDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnBuildingDeath, ABuilding*, Building );

UCLASS( Abstract )
class LORDS_FRONTIERS_API ABuilding : public APawn, public IEntity, public ISelectable
{
	GENERATED_BODY()

public:
	ABuilding();

	bool IsDestroyed() const;

	virtual void RestoreFromRuins();

	virtual void FullRestore();

	virtual FString GetNameBuild();

	virtual FEntityStats& Stats() override;

	virtual const FEntityStats& Stats() const override;

	virtual ETeam Team() override;

	virtual ETeam Team() const override;

	virtual void TakeDamage( int damage ) override;

	virtual void OnSelected_Implementation() override;

	virtual void OnDeselected_Implementation() override;

	virtual bool CanBeSelected_Implementation() const override;

	virtual FVector GetSelectionLocation_Implementation() const override;

	TObjectPtr<UStaticMesh> GetBuildingMesh() const
	{
		return BuildingMesh_;
	}

	const FResourceProduction& GetMaintenanceCost() const
	{
		return MaintenanceCost_;
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

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|UI" )
	TObjectPtr<UTexture2D> BuildingIcon;

	UPROPERTY( BlueprintAssignable )
	FOnBuildingDeath OnBuildingDied;

	static UTexture2D* GetBuildingIconFromClass( TSubclassOf<ABuilding> buildingClass );
	UFUNCTION( BlueprintPure, Category = "Settings|State" )
	bool IsRuined() const
	{
		return bIsRuined_;
	}

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay( const EEndPlayReason::Type endPlayReason ) override;

	virtual UNiagaraSystem* GetHitVFX() const override;

	void ResolveVFXDefaults();

	void SpawnDestructionVFX();

	virtual void OnDeath();

	void FinalizeRuin();
	
	void SubscribeHealthBar();

	void UnsubscribeHealthBar();

	void ActivateBuildingMesh();

	void ActivateRuinsMesh();

	UPROPERTY()
	TObjectPtr<UBoxComponent> CollisionComponent_;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly )
	USkeletalMeshComponent* SkeletalMeshComponent_;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly )
	UStaticMeshComponent* StaticMeshComponent_;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Visuals" )
	TObjectPtr<UStaticMesh> RuinedMesh_;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Visuals" )
	TObjectPtr<UStaticMesh> BuildingMesh_;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Visuals" )
	FVector2D AnimationRateRange_ = FVector2D( 0.8f, 1.2f );

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|State" )
	bool bIsRuined_ = false;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Stats" )
	FEntityStats Stats_;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|HealthBar" )
	TObjectPtr<UHealthBarConfigDataAsset> HealthBarConfig_;

	FDelegateHandle HealthBarSubscription_;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Economy" )
	FResourceProduction BuildingCost_;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Economy" )
	FResourceProduction MaintenanceCost_;

	UPROPERTY()
	TObjectPtr<UEconomyComponent> EconomyComponent_;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> DestructionVFX_;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> HitVFX_;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> ResolvedHitVFX_;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> ResolvedDestructionVFX_;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> ResolvedConstructionVFX_;

	float ResolvedRuinDelay_ = 0.0f;
	float ResolvedConstructionDelay_ = 0.0f;

	void SpawnConstructionVFX();

private:
	FTimerHandle RuinTimerHandle_;
	FTimerHandle ConstructionVFXTimerHandle_;


	FResourceProduction OriginalMaintenanceCost_;

	UPROPERTY( EditAnywhere, Category = "Settings|Visuals" )
	FText BuildingDisplayName_;
};
