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

	/**
	 * Called by the owning actor when it dies. Clears every active status
	 * (running OnRemove and tearing down sticky visuals) and stops the
	 * tracker tick so DOT/visuals don't persist on the corpse.
	 *
	 * Safe to call multiple times.
	 */
	UFUNCTION( BlueprintCallable, Category = "Status" )
	void NotifyOwnerDied();

	static UStatusEffectTracker* EnsureOn( AActor* actor );

private:
	void ReleaseStatusVisual( FActiveStatus& state );

	UPROPERTY()
	TMap<FName, FActiveStatus> Active_;
};
