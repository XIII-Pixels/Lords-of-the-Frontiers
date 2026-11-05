#pragma once

#include "Cards/Visuals/CardVisualTypes.h"

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "StatusEffectTracker.generated.h"

class UStatusEffectDef;

USTRUCT()
struct FActiveStatus
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UStatusEffectDef> Def = nullptr;

	float ExpiresAt = 0.f;
	float NextTickAt = 0.f;
	float CachedOriginal = 0.f;
	float StackAmount = 0.f;

	UPROPERTY()
	TWeakObjectPtr<AActor> Instigator;

	UPROPERTY()
	FCardVisualHandle VisualHandle;
};

UCLASS( ClassGroup = ( Cards ), meta = ( BlueprintSpawnableComponent ) )
class LORDS_FRONTIERS_API UStatusEffectTracker : public UActorComponent
{
	GENERATED_BODY()

public:
	UStatusEffectTracker();

	virtual void TickComponent( float dt, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction ) override;
	virtual void EndPlay( const EEndPlayReason::Type endPlayReason ) override;

	UFUNCTION( BlueprintCallable, Category = "Status" )
	void ApplyStatus( UStatusEffectDef* def, AActor* instigator = nullptr );

	UFUNCTION( BlueprintPure, Category = "Status" )
	bool HasStatus( const UStatusEffectDef* def ) const;

	UFUNCTION( BlueprintPure, Category = "Status" )
	bool HasStatusTag( FName tag ) const;

	UFUNCTION( BlueprintCallable, Category = "Status" )
	void RemoveStatus( UStatusEffectDef* def );

	UFUNCTION( BlueprintCallable, Category = "Status" )
	void ClearAll();

	UFUNCTION( BlueprintCallable, Category = "Status" )
	void NotifyOwnerDied();

	static UStatusEffectTracker* EnsureOn( AActor* actor );

private:
	void ReleaseStatusVisual( FActiveStatus& state );
	void RecomputeStackedModifiers();

	UPROPERTY()
	TArray<FActiveStatus> Active_;

	// Deltas currently applied by slows. Removing a slow subtracts exactly these deltas,
	// so stat changes made by cards while the slow was active are preserved.
	float AppliedMaxSpeedDelta_ = 0.f;
	float AppliedAttackCooldownDelta_ = 0.f;
};
