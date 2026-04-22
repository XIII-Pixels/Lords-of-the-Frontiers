#pragma once

#include "Cards/CardTypes.h"

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "CardEffectHostComponent.generated.h"

class UCardDataAsset;
class UCardEffect;
class UAttackRangedComponent;

USTRUCT()
struct FRegisteredCardEffect
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UCardDataAsset> SourceCard = nullptr;

	UPROPERTY()
	int32 EventIndex = INDEX_NONE;

	UPROPERTY()
	TObjectPtr<UCardEffect> Effect = nullptr;
};

UCLASS( ClassGroup = ( Cards ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API UCardEffectHostComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCardEffectHostComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay( const EEndPlayReason::Type endPlayReason ) override;

	void RegisterEffect( UCardDataAsset* card, int32 eventIndex, UCardEffect* effect );
	void UnregisterBySourceCard( UCardDataAsset* card );
	void ClearAll();

	const TArray<FRegisteredCardEffect>& GetActiveEffects() const
	{
		return Active_;
	}

	UFUNCTION( BlueprintCallable, Category = "Card|Counters" )
	int32 GetCounter( FName key ) const;

	UFUNCTION( BlueprintCallable, Category = "Card|Counters" )
	void SetCounter( FName key, int32 value );

	UFUNCTION( BlueprintCallable, Category = "Card|Counters" )
	int32 IncrementCounter( FName key );

	UFUNCTION( BlueprintPure, Category = "Card|Counters" )
	static FName MakeCounterKey( const UCardDataAsset* card, int32 eventIndex, FName localTag );

protected:
	void BindAttackDelegates();
	void UnbindAttackDelegates();

	void BindBuildingDelegates();
	void UnbindBuildingDelegates();

	void BindDamageEvents();
	void UnbindDamageEvents();

	UFUNCTION()
	void HandleAttackFired( AActor* target );

	UFUNCTION()
	void HandleAttackTargetChanged( AActor* oldTarget, AActor* newTarget );

	UFUNCTION()
	void HandleOwnerDamaged( class ABuilding* building, int32 damage, AActor* instigator );

	UFUNCTION()
	void HandleOwnerRuined( class ABuilding* building );

	void HandleDamageDealt( AActor* instigator, AActor* target, int damage, bool bIsSplash );
	void HandleProjectileMissed( AActor* instigator );

	UFUNCTION()
	void HandleAuraTick();

	void DispatchTrigger( ECardTriggerReason reason, AActor* instigator );

	void StartAuraTimer();
	void StopAuraTimer();

private:
	UPROPERTY()
	TArray<FRegisteredCardEffect> Active_;

	UPROPERTY()
	TMap<FName, int32> Counters_;

	TWeakObjectPtr<UAttackRangedComponent> BoundAttack_;
	bool bIsBoundToAttack_ = false;

	bool bIsBoundToBuilding_ = false;

	FDelegateHandle DamageEventHandle_;
	FDelegateHandle MissEventHandle_;
	bool bIsBoundToDamageEvents_ = false;

	FTimerHandle AuraTimerHandle_;
	bool bIsAuraTicking_ = false;

	UPROPERTY( EditAnywhere, Category = "Settings|Cards" )
	float AuraTickIntervalSeconds_ = 0.5f;
};
