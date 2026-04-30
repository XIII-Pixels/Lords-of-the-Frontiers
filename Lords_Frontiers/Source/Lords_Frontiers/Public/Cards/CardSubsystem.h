#pragma once

#include "Cards/CardTypes.h"

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "CardSubsystem.generated.h"

class UCardDataAsset;
class UCardPoolConfig;
class UCardEffect;
class ABuilding;
class UGameLoopManager;
enum class EGameLoopPhase : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnCardSelectionRequired, const FCardChoice&, Choice );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnCardsApplied, const TArray<UCardDataAsset*>&, AppliedCards );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnEconomyBonusesChanged, const FEconomyBonuses&, NewBonuses );

UCLASS()
class LORDS_FRONTIERS_API UCardSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintPure, Category = "Cards", meta = ( WorldContext = "worldContextObject" ) )
	static UCardSubsystem* Get( const UObject* worldContextObject );

	virtual void Initialize( FSubsystemCollectionBase& collection ) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem( UObject* outer ) const override
	{
		return true;
	}

	UFUNCTION( BlueprintCallable, Category = "Cards|Setup" )
	void SetPoolConfig( UCardPoolConfig* config );

	UFUNCTION( BlueprintPure, Category = "Cards|Setup" )
	UCardPoolConfig* GetPoolConfig() const
	{
		return PoolConfig_;
	}

	UFUNCTION( BlueprintCallable, Category = "Cards|Selection" )
	void RequestCardSelection( int32 waveNumber );

	UFUNCTION( BlueprintCallable, Category = "Cards|Selection" )
	void ApplySelectedCards( const TArray<UCardDataAsset*>& selectedCards );

	UFUNCTION( BlueprintCallable, Category = "Cards|Selection" )
	void ApplySingleCard( UCardDataAsset* card, int32 waveNumber );

	UFUNCTION( BlueprintPure, Category = "Cards|Economy" )
	const FEconomyBonuses& GetEconomyBonuses() const
	{
		return EconomyBonuses_;
	}

	UFUNCTION( BlueprintPure, Category = "Cards|Economy" )
	int32 GetProductionBonus( EResourceTargetType resourceType ) const;

	UFUNCTION( BlueprintPure, Category = "Cards|Economy" )
	int32 GetMaintenanceCostReduction( EResourceTargetType resourceType ) const;

	void AddEconomyProductionBonus( EResourceTargetType target, int32 delta );
	void AddEconomyMaintenanceReduction( EResourceTargetType target, int32 delta );

	UFUNCTION( BlueprintPure, Category = "Cards|History" )
	const TArray<FAppliedCardRecord>& GetAppliedCardHistory() const
	{
		return AppliedCardHistory_;
	}

	UFUNCTION( BlueprintPure, Category = "Cards|History" )
	const TArray<UCardDataAsset*>& GetAcquisitionLog() const
	{
		return AcquisitionLog_;
	}

	UFUNCTION( BlueprintPure, Category = "Cards|History" )
	int32 GetCardStackCount( const UCardDataAsset* card ) const;

	UFUNCTION( BlueprintCallable, Category = "Cards|History" )
	void ResetCardHistory();

	UFUNCTION( BlueprintPure, Category = "Cards|Unlocks" )
	bool IsCardUnlocked( const UCardDataAsset* card ) const;

	UFUNCTION( BlueprintCallable, Category = "Cards|Unlocks" )
	void UnlockCard( UCardDataAsset* card );

	UFUNCTION( BlueprintCallable, Category = "Cards|Unlocks" )
	void LockCard( UCardDataAsset* card );

	UFUNCTION( BlueprintCallable, Category = "Cards|Unlocks" )
	void UnlockCardByID( FName cardID );

	UFUNCTION( BlueprintCallable, Category = "Cards|Unlocks" )
	void LockCardByID( FName cardID );

	UFUNCTION( BlueprintCallable, Category = "Cards|Unlocks" )
	void ResetUnlocksToDefaults();

	UFUNCTION( BlueprintPure, Category = "Cards|Unlocks" )
	TArray<UCardDataAsset*> GetUnlockedCards() const;

	UFUNCTION( BlueprintPure, Category = "Cards|Buildings" )
	TArray<FAppliedCardBonus> GetBuildingBonuses( const ABuilding* building ) const;

	UFUNCTION( BlueprintCallable, Category = "Cards|Buildings" )
	void OnBuildingPlaced( ABuilding* building );

	UFUNCTION( BlueprintCallable, Category = "Cards|Debug" )
	void DebugApplyCardByID( FName cardID );

	UFUNCTION( BlueprintCallable, Category = "Cards|Debug" )
	void ToggleDebugShowAll();

	UPROPERTY( BlueprintAssignable, Category = "Cards|Events" )
	FOnCardSelectionRequired OnCardSelectionRequired;

	UPROPERTY( BlueprintAssignable, Category = "Cards|Events" )
	FOnCardsApplied OnCardsApplied;

	UPROPERTY( BlueprintAssignable, Category = "Cards|Events" )
	FOnEconomyBonusesChanged OnEconomyBonusesChanged;

protected:
	void ApplySingleCardOnce( UCardDataAsset* card, int32 waveNumber, const TArray<ABuilding*>& buildings );

	void ApplyCardEvent(
		UCardDataAsset* card, int32 eventIndex, const FCardEvent& event,
		int32 waveNumber, int32 stackCount,
		const TArray<ABuilding*>& buildings );

	FCardEffectContext MakeContext(
		UCardDataAsset* card, int32 eventIndex, int32 stackCount,
		int32 waveNumber, ABuilding* building );

	bool EvaluateConditions( const FCardEvent& event, const FCardEffectContext& context ) const;

	void RevertAppliedRecord( const FAppliedCardRecord& record );

	class UCardEffectHostComponent* EnsureEffectHost( ABuilding* building ) const;

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
	TArray<TObjectPtr<UCardDataAsset>> AcquisitionLog_;

	UPROPERTY()
	TSet<TObjectPtr<UCardDataAsset>> RuntimeUnlocked_;

	UPROPERTY()
	TSet<TObjectPtr<UCardDataAsset>> RuntimeLocked_;

	UPROPERTY()
	FEconomyBonuses EconomyBonuses_;

	int32 CurrentWaveNumber_ = 0;

	bool bIsBoundToGameLoop_ = false;

	TWeakObjectPtr<UGameLoopManager> CachedGameLoop_;
};
