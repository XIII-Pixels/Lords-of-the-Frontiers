#pragma once

#include "Building/Bonus/BuildingBonusComponent.h"
#include "Entity.h"
#include "EntityStats.h"
#include "Lords_Frontiers/Public/Resources/GameResource.h"
#include "Selectable.h"

#include "CoreMinimal.h"
#include "sound/AudioEvent.h"
#include "sound/AudioEventSource.h"

#include "Building.generated.h"

class UEconomyComponent;
class UBoxComponent;
class UNiagaraSystem;
class UHealthBarConfigDataAsset;
class UGeometryCacheComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnBuildingDeath, ABuilding*, Building );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnBuildingRestored, ABuilding*, Building );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnBuildingDamaged, ABuilding*, Building, int32, Damage, AActor*, Instigator
);

USTRUCT( BlueprintType )
struct FBuildingAudioTags
{
	GENERATED_BODY()

	UPROPERTY( EditDefaultsOnly )
	FGameplayTag Selected;

	UPROPERTY( EditDefaultsOnly )
	FGameplayTag PlacedSuccess;

	UPROPERTY( EditDefaultsOnly )
	FGameplayTag PlacedRestricted;

	UPROPERTY( EditDefaultsOnly )
	FGameplayTag Demolished;

	UPROPERTY( EditDefaultsOnly )
	FGameplayTag Death;

	UPROPERTY( EditDefaultsOnly )
	FGameplayTag Resurrected;

	UPROPERTY( EditDefaultsOnly )
	FGameplayTag Attack;

	UPROPERTY( EditDefaultsOnly )
	FGameplayTag TakeDamage;
};

UCLASS( Abstract )
class LORDS_FRONTIERS_API ABuilding : public APawn, public IEntity, public ISelectable, public IAudioEventSource
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

	virtual void TakeDamage( int damage, AActor* instigator = nullptr ) override;

	virtual void OnSelected_Implementation() override;

	virtual void OnDeselected_Implementation() override;

	virtual bool CanBeSelected_Implementation() const override;

	virtual FVector GetSelectionLocation_Implementation() const override;

	TObjectPtr<UStaticMesh> GetBuildingMesh() const
	{
		return BuildingMesh_;
	}

	TObjectPtr<UStaticMesh> GetPreviewMesh() const
	{
		if ( !PreviewMesh_ )
		{
			return BuildingMesh_;
		}
		return PreviewMesh_;
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

	UPROPERTY( BlueprintAssignable )
	FOnBuildingRestored OnBuildingRestored;

	UPROPERTY( BlueprintAssignable )
	FOnBuildingDamaged OnBuildingDamaged;

	static UTexture2D* GetBuildingIconFromClass( TSubclassOf<ABuilding> buildingClass );

	UFUNCTION( BlueprintPure, Category = "Settings|State" )
	bool IsRuined() const
	{
		return bIsRuined_;
	}

	UFUNCTION( BlueprintPure, Category = "Settings|Build" )
	bool CanBeRelocated() const
	{
		return bCanBeRelocated_;
	}

	UFUNCTION( BlueprintPure, Category = "Settings|Build" )
	bool CanBeRemoved() const
	{
		return bCanBeRemoved_;
	}

	virtual FOnAudioEvent& GetOnAudioEvent() override
	{
		return OnAudioEvent_;
	}

	UFUNCTION( BlueprintPure, Category = "Settings|Economy" )
	int32 GetBuildingTotalCostGold() const;

	UFUNCTION( BlueprintPure, Category = "Settings|Economy" )
	int32 GetRelocationGoldCost() const;

	UFUNCTION( BlueprintPure, Category = "Settings|Economy" )
	FResourceProduction GetRelocationCost() const;

	UFUNCTION( BlueprintPure, Category = "Settings|Economy" )
	FResourceProduction GetDemolitionRefund() const;

	const FBuildingAudioTags& AudioTags() const
	{
		return AudioTags_;
	}

	void SpawnConstructionVFX();

	void SpawnDestructionVFX();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay( const EEndPlayReason::Type endPlayReason ) override;

	virtual void PostInitProperties() override;

	virtual UNiagaraSystem* GetHitVFX() const override;

	void ResolveVFXDefaults();

	virtual void OnDeath();

	void FinalizeRuin();

	void SubscribeHealthBar();

	void UnsubscribeHealthBar();

	void ActivateBuildingMesh();

	void ActivateRuinsMesh();

	void SetStaticMeshWithMaterials( UStaticMesh* mesh );

	void UpdateSelectionOverlay();

	void ShowSelectionOverlay();

	void HideSelectionOverlay();

	void ResolveAudioTags();

	UPROPERTY()
	TObjectPtr<UBoxComponent> CollisionComponent_;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly )
	USkeletalMeshComponent* SkeletalMeshComponent_;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly )
	UStaticMeshComponent* StaticMeshComponent_;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly )
	UGeometryCacheComponent* GeometryCacheComponent_;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Visuals" )
	TObjectPtr<UStaticMesh> RuinedMesh_;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Visuals" )
	TObjectPtr<UStaticMesh> BuildingMesh_;

	UPROPERTY(
	    EditAnywhere, BlueprintReadOnly, Category = "Settings|Visuals", meta = ( ToolTip = "Defaults to Building Mesh" )
	)
	TObjectPtr<UStaticMesh> PreviewMesh_;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Visuals" )
	TObjectPtr<UMaterialInterface> SelectionMaterial_;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Visuals" )
	TObjectPtr<UStaticMeshComponent> SelectionOverlayMesh_;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Settings|Visuals" )
	FVector2D AnimationRateRange_ = FVector2D( 0.8f, 1.2f );

	UPROPERTY( EditDefaultsOnly, Category = "Settings|Audio" )
	FBuildingAudioTags AudioTags_;

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

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Build" )
	bool bCanBeRelocated_ = true;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Build" )
	bool bCanBeRemoved_ = true;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Build" )
	FResourceProduction RelocationCost_;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Build" )
	FResourceProduction DemolitionRefund_;

	FOnAudioEvent OnAudioEvent_;

private:
	FTimerHandle RuinTimerHandle_;
	FTimerHandle ConstructionVFXTimerHandle_;

	FResourceProduction OriginalMaintenanceCost_;

	UPROPERTY( EditAnywhere, Category = "Settings|Visuals" )
	FText BuildingDisplayName_;
};
