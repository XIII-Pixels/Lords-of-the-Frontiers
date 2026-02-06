#pragma once

#include "Cards/CardTypes.h"

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "CardSubsystem.generated.h"

class UCardDataAsset;
class UCardPoolConfig;
class ABuilding;
class AResourceBuilding;
class UGameLoopManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnCardSelectionRequired, const FCardChoice&, Choice );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnCardsApplied, const TArray<UCardDataAsset*>&, AppliedCards );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnEconomyBonusesChanged, const FEconomyBonuses&, NewBonuses );

/**
 * UCardSubsystem
 *
 * Central manager for the card system.
 * Handles card selection, application, and history tracking.
 *
 * Two modifier categories with smart routing:
 *   1. Building Stats — modify FEntityStats on matching buildings
 *   2. Resource Modifiers (MaintenanceCost, Production):
 *      - TargetFilter == Any  → global economy bonus (FEconomyBonuses)
 *      - TargetFilter != Any  → directly on matching buildings
 */
UCLASS()
class LORDS_FRONTIERS_API UCardSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintPure, Category = "Cards", meta = ( WorldContext = "worldContextObject" ) )
	static UCardSubsystem* Get( const UObject* worldContextObject );

	virtual void Initialize( FSubsystemCollectionBase& collection ) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem( UObject* outer ) const override { return true; }

	UFUNCTION( BlueprintCallable, Category = "Cards|Setup" )
	void SetPoolConfig( UCardPoolConfig* config );

	UFUNCTION( BlueprintPure, Category = "Cards|Setup" )
	UCardPoolConfig* GetPoolConfig() const { return PoolConfig_; }

	UFUNCTION( BlueprintCallable, Category = "Cards|Selection" )
	void RequestCardSelection( int32 waveNumber );

	UFUNCTION( BlueprintCallable, Category = "Cards|Selection" )
	void ApplySelectedCards( const TArray<UCardDataAsset*>& selectedCards );

	UFUNCTION( BlueprintCallable, Category = "Cards|Selection" )
	void ApplySingleCard( UCardDataAsset* card, int32 waveNumber );

	UFUNCTION( BlueprintPure, Category = "Cards|Economy" )
	const FEconomyBonuses& GetEconomyBonuses() const { return EconomyBonuses_; }

	UFUNCTION( BlueprintPure, Category = "Cards|Economy" )
	int32 GetProductionBonus( EResourceTargetType resourceType ) const;

	UFUNCTION( BlueprintPure, Category = "Cards|Economy" )
	int32 GetMaintenanceCostReduction( EResourceTargetType resourceType ) const;

	UFUNCTION( BlueprintPure, Category = "Cards|History" )
	const TArray<FAppliedCardRecord>& GetAppliedCardHistory() const { return AppliedCardHistory_; }

	UFUNCTION( BlueprintPure, Category = "Cards|History" )
	int32 GetCardStackCount( const UCardDataAsset* card ) const;

	UFUNCTION( BlueprintCallable, Category = "Cards|History" )
	void ResetCardHistory();

	UFUNCTION( BlueprintPure, Category = "Cards|Buildings" )
	TArray<FAppliedCardBonus> GetBuildingBonuses( const ABuilding* building ) const;

	UFUNCTION( BlueprintCallable, Category = "Cards|Buildings" )
	void OnBuildingPlaced( ABuilding* building );

	UPROPERTY( BlueprintAssignable, Category = "Cards|Events" )
	FOnCardSelectionRequired OnCardSelectionRequired;

	UPROPERTY( BlueprintAssignable, Category = "Cards|Events" )
	FOnCardsApplied OnCardsApplied;

	UPROPERTY( BlueprintAssignable, Category = "Cards|Events" )
	FOnEconomyBonusesChanged OnEconomyBonusesChanged;

protected:
	TArray<UCardDataAsset*> GenerateCardSelection( int32 count );

	/**
	 * Internal implementation of card application.
	 * Accepts pre-cached buildings array to avoid repeated TActorIterator calls.
	 */
	void ApplySingleCardInternal(
		UCardDataAsset* card, int32 waveNumber, const TArray<ABuilding*>& buildings );

	/** Applies all building-targeted modifiers from card (skips global resource mods) */
	void ApplyCardToBuilding( UCardDataAsset* card, ABuilding* building );

	/** Routes a single modifier to the correct handler */
	void ApplyModifierToBuilding( const FCardStatModifier& modifier, ABuilding* building );

	/** Applies MaintenanceCost modifier directly to building */
	void ApplyMaintenanceCostModifier( const FCardStatModifier& modifier, ABuilding* building );

	/** Applies Production modifier (only to AResourceBuilding) */
	void ApplyProductionModifier( const FCardStatModifier& modifier, ABuilding* building );

	/** Applies FEntityStats modifiers (MaxHealth, AttackDamage, etc.) */
	void ApplyEntityStatModifier( const FCardStatModifier& modifier, ABuilding* building );

	/** Applies resource modifiers as global economy bonuses + broadcasts */
	void ApplyGlobalEconomyModifiers( UCardDataAsset* card, int32 stackCount = 1 );

	/**
	 * Applies resource modifiers as global economy bonuses without broadcasting.
	 * Used internally by RecalculateEconomyBonuses to batch changes.
	 */
	void ApplyGlobalEconomyModifiersInternal( UCardDataAsset* card, int32 stackCount );

	void RecalculateEconomyBonuses();

	/**
	 * Reverts all building stat/direct modifiers by applying inverse values.
	 * Called before clearing history to keep buildings in sync.
	 */
	void RevertAllBuildingModifiers();

	TArray<ABuilding*> GetAllBuildings() const;

	void BindToGameLoop();
	void UnbindFromGameLoop();

	UFUNCTION()
	void HandlePhaseChanged( EGameLoopPhase oldPhase, EGameLoopPhase newPhase );

	void NotifyGameLoopToProceed();

	UWorld* GetWorldSafe() const;

private:
	UPROPERTY()
	TObjectPtr<UCardPoolConfig> PoolConfig_;

	UPROPERTY()
	TArray<FAppliedCardRecord> AppliedCardHistory_;

	UPROPERTY()
	FEconomyBonuses EconomyBonuses_;

	int32 CurrentWaveNumber_ = 0;

	bool bIsBoundToGameLoop_ = false;

	TWeakObjectPtr<UGameLoopManager> CachedGameLoop_;
};
